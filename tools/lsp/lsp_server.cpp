#include "json.h"
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <optional>
#include <filesystem>
#include <algorithm>
#include <fstream>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "lexer/token.h"
#include "parser/ast.h"
#include "parser/stmt.h"
#include "interpreter/interpreter.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "formatter.h"

namespace claw::lsp {

struct Position { int line = 0; int character = 0; };
struct Range { Position start; Position end; };

struct SymbolInfo {
    std::string name;
    Range def;
    std::vector<Range> refs;
    std::string typeName;
    std::vector<std::string> params;
    int arity = 0;
};

struct Document {
    std::string uri;
    std::string path;
    std::string text;
    std::vector<Token> tokens;
    std::vector<StmtPtr> statements;
    std::vector<std::string> parserErrors;
    std::unordered_map<std::string, SymbolInfo> symbols;
    std::unordered_map<std::string, SymbolInfo> locals;
    std::vector<Range> unknownRefs;
    std::vector<std::pair<Range, std::string>> styleIssues;
    std::unordered_map<std::string, std::vector<Range>> refByName;
};

static std::unordered_map<std::string, Document> g_docs;
static std::shared_ptr<claw::Interpreter> g_interp;
static std::unordered_map<std::string, Document> g_wsdocs;
static std::string g_workspaceRoot;

static std::string pathToUri(const std::string& path) {
    std::string p = path;
    for (auto& c : p) if (c == '\\') c = '/';
    if (p.rfind("file://", 0) == 0) return p;
    return std::string("file:///") + p;
}

static std::string uriToPath(const std::string& uri) {
    if (uri.rfind("file:///", 0) == 0) {
        std::string p = uri.substr(8);
        for (auto& c : p) if (c == '/') c = '\\';
        return p;
    }
    if (uri.rfind("file://", 0) == 0) return uri.substr(7);
    return uri;
}

static std::string readHeaderLine() {
    std::string line;
    std::getline(std::cin, line);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

static std::optional<std::string> readMessage() {
    std::string header;
    size_t contentLength = 0;
    while (true) {
        header = readHeaderLine();
        if (header.empty()) break;
        if (header.rfind("Content-Length:", 0) == 0) {
            std::string value = header.substr(15);
            value.erase(0, value.find_first_not_of(" \t"));
            contentLength = static_cast<size_t>(std::stoul(value));
        }
    }
    if (contentLength == 0) return std::nullopt;
    std::string body(contentLength, '\0');
    std::cin.read(body.data(), contentLength);
    return body;
}

static void writeMessage(const Json& j) {
    std::string payload = stringify(j);
    std::ostringstream oss;
    oss << "Content-Length: " << payload.size() << "\r\n\r\n" << payload;
    std::cout << oss.str();
    std::cout.flush();
}

static Range tokenRange(const Token& t) {
    Range r;
    r.start.line = std::max(0, t.line - 1);
    r.start.character = std::max(0, t.column - 1);
    r.end.line = r.start.line;
    r.end.character = r.start.character + static_cast<int>(t.lexeme.size());
    return r;
}

static void analyzeDocument(Document& doc) {
    Lexer lexer(doc.text);
    doc.tokens = lexer.tokenize();
    Parser parser(doc.tokens);
    doc.statements = parser.parseProgram();
    doc.parserErrors = parser.getErrors();
    doc.symbols.clear();
    doc.locals.clear();
    doc.unknownRefs.clear();
    doc.styleIssues.clear();
    for (const auto& s : doc.statements) {
        if (auto* let = dynamic_cast<LetStmt*>(s.get())) {
            SymbolInfo si;
            si.name = std::string(let->name);
            si.def = tokenRange(let->token);
            si.typeName = "unknown";
            if (let->initializer) {
                if (auto* lit = dynamic_cast<LiteralExpr*>(let->initializer.get())) {
                    switch (lit->type) {
                        case LiteralExpr::Type::Number: si.typeName = "number"; break;
                        case LiteralExpr::Type::String: si.typeName = "string"; break;
                        case LiteralExpr::Type::Bool: si.typeName = "bool"; break;
                        case LiteralExpr::Type::Nil: si.typeName = "nil"; break;
                    }
                } else if (auto* fexpr = dynamic_cast<FunctionExpr*>(let->initializer.get())) {
                    si.typeName = "function";
                    si.arity = static_cast<int>(fexpr->parameters.size());
                    for (auto& p : fexpr->parameters) si.params.push_back(p);
                }
            }
            doc.symbols[si.name] = std::move(si);
        } else if (auto* fn = dynamic_cast<FnStmt*>(s.get())) {
            SymbolInfo si;
            si.name = fn->name;
            si.def = tokenRange(fn->token);
            si.typeName = "function";
            si.arity = static_cast<int>(fn->parameters.size());
            for (auto& p : fn->parameters) si.params.push_back(p);
            doc.symbols[si.name] = std::move(si);
        } else if (auto* cls = dynamic_cast<ClassStmt*>(s.get())) {
            SymbolInfo si;
            si.name = cls->name;
            si.def = tokenRange(cls->token);
            si.typeName = "class";
            doc.symbols[si.name] = std::move(si);
        }
    }
    struct LocalCollector {
        Document* d;
        void walk(Stmt* s) {
            if (!s) return;
            if (auto* l = dynamic_cast<LetStmt*>(s)) {
                SymbolInfo si;
                si.name = std::string(l->name);
                si.def = tokenRange(l->token);
                d->locals[si.name] = std::move(si);
                if (l->initializer) walk(l->initializer.get());
                return;
            }
            if (auto* b = dynamic_cast<BlockStmt*>(s)) {
                for (auto& st : b->statements) walk(st.get());
                return;
            }
            if (auto* fn = dynamic_cast<FnStmt*>(s)) {
                for (auto& p : fn->parameters) {
                    SymbolInfo si;
                    si.name = p;
                    d->locals[si.name] = std::move(si);
                }
                for (auto& st : fn->body) walk(st.get());
                return;
            }
            if (auto* c = dynamic_cast<ClassStmt*>(s)) {
                for (auto& m : c->methods) walk(m.get());
                return;
            }
            if (auto* e = dynamic_cast<ExprStmt*>(s)) {
                if (e->expr) walk(e->expr.get());
                return;
            }
        }
        void walk(Expr* e) {
            if (!e) return;
            if (auto* g = dynamic_cast<GroupingExpr*>(e)) { walk(g->expr.get()); return; }
            if (auto* a = dynamic_cast<ArrayExpr*>(e)) { for (auto& el : a->elements) walk(el.get()); return; }
            if (auto* h = dynamic_cast<HashMapExpr*>(e)) { for (auto& kv : h->keyValuePairs) { walk(kv.first.get()); walk(kv.second.get()); } return; }
            if (auto* b = dynamic_cast<BinaryExpr*>(e)) { walk(b->left.get()); walk(b->right.get()); return; }
            if (auto* l = dynamic_cast<LogicalExpr*>(e)) { walk(l->left.get()); walk(l->right.get()); return; }
            if (auto* i = dynamic_cast<IndexExpr*>(e)) { walk(i->object.get()); walk(i->index.get()); return; }
            if (auto* m = dynamic_cast<MemberExpr*>(e)) { walk(m->object.get()); return; }
            if (auto* c = dynamic_cast<CallExpr*>(e)) { walk(c->callee.get()); for (auto& arg : c->arguments) walk(arg.get()); return; }
            if (auto* t = dynamic_cast<TernaryExpr*>(e)) { walk(t->condition.get()); walk(t->thenBranch.get()); walk(t->elseBranch.get()); return; }
        }
    } lc{&doc};
    for (const auto& s : doc.statements) lc.walk(s.get());
    for (const auto& s : doc.statements) {
        struct Walker {
            Document* d;
            void walk(Expr* e) {
                if (!e) return;
                if (auto* v = dynamic_cast<VariableExpr*>(e)) {
                    auto it = d->symbols.find(v->name);
                    if (it != d->symbols.end()) {
                        it->second.refs.push_back(tokenRange(v->token));
                    } else {
                        if (d->locals.find(v->name) != d->locals.end()) {
                            return;
                        }
                        bool isBuiltin = false;
                        if (g_interp) {
                            try {
                                auto g = g_interp->getGlobals();
                                isBuiltin = g->exists(v->name);
                            } catch (...) {}
                        }
                        if (!isBuiltin) {
                            d->unknownRefs.push_back(tokenRange(v->token));
                            d->refByName[std::string(v->name)].push_back(tokenRange(v->token));
                        } else {
                            d->refByName[std::string(v->name)].push_back(tokenRange(v->token));
                        }
                    }
                    return;
                }
                if (auto* a = dynamic_cast<AssignExpr*>(e)) {
                    walk(a->value.get());
                    if (auto it = d->symbols.find(a->name); it != d->symbols.end()) {
                        it->second.refs.push_back(tokenRange(a->token));
                    }
                    d->refByName[std::string(a->name)].push_back(tokenRange(a->token));
                    return;
                }
                if (auto* b = dynamic_cast<BinaryExpr*>(e)) {
                    walk(b->left.get());
                    walk(b->right.get());
                    return;
                }
                if (auto* c = dynamic_cast<CallExpr*>(e)) {
                    walk(c->callee.get());
                    for (auto& arg : c->arguments) walk(arg.get());
                    if (auto* ce = dynamic_cast<CallExpr*>(e)) {
                        if (auto* calleeVar = dynamic_cast<VariableExpr*>(ce->callee.get())) {
                            d->refByName[std::string(calleeVar->name)].push_back(tokenRange(calleeVar->token));
                        }
                    }
                    return;
                }
                if (auto* g = dynamic_cast<GroupingExpr*>(e)) { walk(g->expr.get()); return; }
                if (auto* l = dynamic_cast<LogicalExpr*>(e)) { walk(l->left.get()); walk(l->right.get()); return; }
                if (auto* i = dynamic_cast<IndexExpr*>(e)) { walk(i->object.get()); walk(i->index.get()); return; }
                if (auto* m = dynamic_cast<MemberExpr*>(e)) { walk(m->object.get()); return; }
                if (auto* t = dynamic_cast<TernaryExpr*>(e)) { walk(t->condition.get()); walk(t->thenBranch.get()); walk(t->elseBranch.get()); return; }
            }
            void walk(Stmt* s) {
                if (!s) return;
                if (auto* e = dynamic_cast<ExprStmt*>(s)) { walk(e->expr.get()); return; }
                if (auto* p = dynamic_cast<PrintStmt*>(s)) { walk(p->expr.get()); return; }
                if (auto* l = dynamic_cast<LetStmt*>(s)) { walk(l->initializer.get()); return; }
                if (auto* r = dynamic_cast<ReturnStmt*>(s)) { walk(r->value.get()); return; }
                if (auto* i = dynamic_cast<IfStmt*>(s)) { walk(i->condition.get()); walk(i->thenBranch.get()); walk(i->elseBranch.get()); return; }
                if (auto* w = dynamic_cast<WhileStmt*>(s)) { walk(w->condition.get()); walk(w->body.get()); return; }
                if (auto* f = dynamic_cast<ForStmt*>(s)) { walk(f->initializer.get()); walk(f->condition.get()); walk(f->increment.get()); walk(f->body.get()); return; }
                if (auto* b = dynamic_cast<BlockStmt*>(s)) { for (auto& st : b->statements) walk(st.get()); return; }
                if (auto* c = dynamic_cast<ClassStmt*>(s)) { for (auto& m : c->methods) walk(m.get()); return; }
                if (auto* fn = dynamic_cast<FnStmt*>(s)) { for (auto& st : fn->body) walk(st.get()); return; }
            }
        } walker{&doc};
        walker.walk(s.get());
    }
    int lineNo = 0;
    size_t start = 0;
    while (start <= doc.text.size()) {
        size_t nl = doc.text.find('\n', start);
        std::string_view line = nl == std::string::npos ? std::string_view(doc.text).substr(start) 
                                                        : std::string_view(doc.text).substr(start, nl - start);
        size_t pos = line.find('\t');
        while (pos != std::string::npos) {
            Range r; r.start.line = lineNo; r.start.character = static_cast<int>(pos);
            r.end.line = lineNo; r.end.character = r.start.character + 1;
            doc.styleIssues.emplace_back(r, "Style: tab character should be replaced with spaces");
            pos = line.find('\t', pos + 1);
        }
        if (!line.empty()) {
            size_t lastNon = line.find_last_not_of(" \t");
            if (lastNon != std::string::npos && lastNon + 1 < line.size()) {
                Range r; r.start.line = lineNo; r.start.character = static_cast<int>(lastNon + 1);
                r.end.line = lineNo; r.end.character = static_cast<int>(line.size());
                doc.styleIssues.emplace_back(r, "Style: trailing whitespace");
            }
        }
        if (line.size() > 120) {
            Range r; r.start.line = lineNo; r.start.character = 0;
            r.end.line = lineNo; r.end.character = static_cast<int>(line.size());
            doc.styleIssues.emplace_back(r, "Style: line exceeds 120 characters");
        }
        if (nl == std::string::npos) break;
        start = nl + 1;
        lineNo++;
    }
}

static Json makePosition(int line, int character) {
    JsonObject o;
    o["line"] = Json::number(line);
    o["character"] = Json::number(character);
    return Json::object(std::move(o));
}

static Json makeRange(const Range& r) {
    JsonObject o;
    o["start"] = makePosition(r.start.line, r.start.character);
    o["end"] = makePosition(r.end.line, r.end.character);
    return Json::object(std::move(o));
}

static Json makeLocation(const std::string& uri, const Range& r) {
    JsonObject loc;
    loc["uri"] = Json::string(uri);
    loc["range"] = makeRange(r);
    return Json::object(std::move(loc));
}

static void sendDiagnostics(const Document& doc) {
    JsonObject params;
    params["uri"] = Json::string(doc.uri);
    JsonArray diags;
    for (const auto& e : doc.parserErrors) {
        JsonObject d;
        d["severity"] = Json::number(1);
        d["message"] = Json::string(e);
        JsonObject r;
        r["start"] = makePosition(0, 0);
        r["end"] = makePosition(0, 0);
        d["range"] = Json::object(std::move(r));
        diags.push_back(Json::object(std::move(d)));
    }
    for (const auto& r : doc.unknownRefs) {
        JsonObject d;
        d["severity"] = Json::number(2);
        d["message"] = Json::string("Unknown identifier");
        d["range"] = makeRange(r);
        diags.push_back(Json::object(std::move(d)));
    }
    for (const auto& iss : doc.styleIssues) {
        JsonObject d;
        d["severity"] = Json::number(3);
        d["message"] = Json::string(iss.second);
        d["range"] = makeRange(iss.first);
        diags.push_back(Json::object(std::move(d)));
    }
    params["diagnostics"] = Json::array(std::move(diags));
    JsonObject msg;
    msg["jsonrpc"] = Json::string("2.0");
    msg["method"] = Json::string("textDocument/publishDiagnostics");
    msg["params"] = Json::object(std::move(params));
    writeMessage(Json::object(std::move(msg)));
}

static void handleInitialize(const Json& req) {
    JsonObject result;
    JsonObject caps;
    caps["hoverProvider"] = Json::boolean(true);
    caps["definitionProvider"] = Json::boolean(true);
    caps["referencesProvider"] = Json::boolean(true);
    caps["documentSymbolProvider"] = Json::boolean(true);
    caps["workspaceSymbolProvider"] = Json::boolean(true);
    caps["renameProvider"] = Json::boolean(true);
    JsonObject onTypeDoc;
    onTypeDoc["firstTriggerCharacter"] = Json::string(";");
    JsonArray more; more.push_back(Json::string("}")); more.push_back(Json::string(",")); more.push_back(Json::string(":"));
    onTypeDoc["moreTriggerCharacter"] = Json::array(std::move(more));
    caps["documentOnTypeFormattingProvider"] = Json::object(std::move(onTypeDoc));
    JsonObject completion;
    completion["resolveProvider"] = Json::boolean(false);
    JsonArray triggers; triggers.push_back(Json::string(".")); triggers.push_back(Json::string("(")); completion["triggerCharacters"] = Json::array(std::move(triggers));
    caps["completionProvider"] = Json::object(std::move(completion));
    caps["documentFormattingProvider"] = Json::boolean(true);
    caps["documentRangeFormattingProvider"] = Json::boolean(true);
    JsonObject onType;
    JsonArray onTypeTriggers; onTypeTriggers.push_back(Json::string("}")); onTypeTriggers.push_back(Json::string("\n"));
    onType["firstTriggerCharacter"] = Json::string("}");
    onType["moreTriggerCharacter"] = Json::array(std::move(onTypeTriggers));
    caps["onTypeFormattingProvider"] = Json::object(std::move(onType));
    JsonObject sigHelp;
    JsonArray sigTriggers; sigTriggers.push_back(Json::string("(")); sigTriggers.push_back(Json::string(","));
    sigHelp["triggerCharacters"] = Json::array(std::move(sigTriggers));
    caps["signatureHelpProvider"] = Json::object(std::move(sigHelp));
    caps["textDocumentSync"] = Json::number(2);
    result["capabilities"] = Json::object(std::move(caps));
    if (!g_interp) {
        g_interp = std::make_shared<claw::Interpreter>();
    }
    auto pRootUri = getPath(req, {"params", "rootUri"});
    auto pRootPath = getPath(req, {"params", "rootPath"});
    if (pRootUri && (**pRootUri).type == Json::Type::String) {
        g_workspaceRoot = uriToPath((**pRootUri).s);
    } else if (pRootPath && (**pRootPath).type == Json::Type::String) {
        g_workspaceRoot = (**pRootPath).s;
    }
    if (!g_workspaceRoot.empty()) {
        try {
            for (auto& p : std::filesystem::recursive_directory_iterator(g_workspaceRoot)) {
                if (p.is_regular_file()) {
                    auto ext = p.path().extension().string();
                    if (ext == ".claw" || ext == ".volt") {
                        std::ifstream ifs(p.path().string(), std::ios::binary);
                        if (!ifs) continue;
                        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                        Document d;
                        d.path = p.path().string();
                        d.uri = pathToUri(d.path);
                        d.text = content;
                        analyzeDocument(d);
                        g_wsdocs[d.path] = std::move(d);
                    }
                }
            }
        } catch (...) {}
    }
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::object(std::move(result));
    writeMessage(Json::object(std::move(resp)));
}

static void handleDocumentSymbol(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    JsonArray result;
    auto it = g_docs.find(uri);
    if (it != g_docs.end()) {
        for (const auto& [name, sym] : it->second.symbols) {
            JsonObject si;
            si["name"] = Json::string(name);
            int kind = 13;
            if (sym.typeName == "function") kind = 12;
            else if (sym.typeName == "class") kind = 5;
            si["kind"] = Json::number(kind);
            si["location"] = makeLocation(uri, sym.def);
            result.push_back(Json::object(std::move(si)));
        }
    }
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::array(std::move(result));
    writeMessage(Json::object(std::move(resp)));
}

static void handleWorkspaceSymbol(const Json& req) {
    auto pQuery = getPath(req, {"params", "query"});
    std::string query = pQuery ? (**pQuery).s : "";
    JsonArray result;
    auto emitDoc = [&](const Document& doc) {
        for (const auto& [name, sym] : doc.symbols) {
            if (query.empty() || name.find(query) != std::string::npos) {
                JsonObject si;
                si["name"] = Json::string(name);
                int kind = 13;
                if (sym.typeName == "function") kind = 12;
                else if (sym.typeName == "class") kind = 5;
                si["kind"] = Json::number(kind);
                si["location"] = makeLocation(doc.uri, sym.def);
                result.push_back(Json::object(std::move(si)));
            }
        }
    };
    for (const auto& [path, doc] : g_wsdocs) emitDoc(doc);
    for (const auto& [uri, doc] : g_docs) emitDoc(doc);
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::array(std::move(result));
    writeMessage(Json::object(std::move(resp)));
}

static void handleFormatting(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    JsonArray edits;
    auto it = g_docs.find(uri);
    if (it != g_docs.end()) {
        Lexer lex(it->second.text);
        auto toks = lex.tokenize();
        std::string formatted = claw::fmt::formatTokens(toks, 2);
        JsonObject te;
        Range rr; rr.start.line = 0; rr.start.character = 0;
        rr.end.line = static_cast<int>(std::count(it->second.text.begin(), it->second.text.end(), '\n'));
        rr.end.character = 0;
        te["range"] = makeRange(rr);
        te["newText"] = Json::string(formatted);
        edits.push_back(Json::object(std::move(te)));
    }
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::array(std::move(edits));
    writeMessage(Json::object(std::move(resp)));
}

static void handleSignatureHelp(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    auto pPos = getPath(req, {"params", "position"});
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    int line = pPos ? static_cast<int>((**pPos).o.at("line").n) : 0;
    int ch = pPos ? static_cast<int>((**pPos).o.at("character").n) : 0;
    std::string fnName;
    int activeParam = 0;
    auto it = g_docs.find(uri);
    if (it != g_docs.end()) {
        const auto& toks = it->second.tokens;
        int lastLParenCol = -1;
        int identCol = -1;
        for (size_t i = 0; i < toks.size(); ++i) {
            const auto& t = toks[i];
            if (t.line - 1 != line) continue;
            int start = t.column - 1;
            int end = start + static_cast<int>(t.lexeme.size());
            if (start <= ch && t.type == TokenType::LeftParen) {
                if (start > lastLParenCol) lastLParenCol = start;
            }
            if (end <= ch && t.type == TokenType::Identifier) {
                identCol = start;
                fnName = t.lexeme;
            }
        }
        if (lastLParenCol >= 0) {
            for (const auto& t : toks) {
                if (t.line - 1 != line) continue;
                int start = t.column - 1;
                if (start > lastLParenCol && start < ch && t.type == TokenType::Comma) {
                    activeParam++;
                }
            }
        }
    }

    JsonObject sigHelp;
    JsonArray signatures;
    JsonObject sig;
    std::string label = fnName + "(";
    int ar = 0;
    if (!fnName.empty()) {
        // Try document symbols first
        auto it2 = g_docs.find(uri);
        if (it2 != g_docs.end()) {
            auto sit = it2->second.symbols.find(fnName);
            if (sit != it2->second.symbols.end() && sit->second.typeName == "function") {
                for (size_t i = 0; i < sit->second.params.size(); ++i) {
                    label += sit->second.params[i];
                    if (i + 1 < sit->second.params.size()) label += ", ";
                }
                ar = static_cast<int>(sit->second.params.size());
            }
        }
        if (ar == 0 && g_interp) {
            auto g = g_interp->getGlobals();
            if (g->exists(fnName)) {
                Value v = g->get(fnName);
                if (isCallable(v)) {
                    auto fn = asCallable(v);
                    ar = fn ? fn->arity() : 0;
                    for (int i = 0; i < ar; ++i) {
                        label += "arg" + std::to_string(i + 1);
                        if (i + 1 < ar) label += ", ";
                    }
                }
            }
        }
    }
    label += ")";
    sig["label"] = Json::string(label);
    JsonArray parameters;
    for (int i = 0; i < ar; ++i) {
        JsonObject p; p["label"] = Json::string("arg" + std::to_string(i + 1));
        parameters.push_back(Json::object(std::move(p)));
    }
    sig["parameters"] = Json::array(std::move(parameters));
    signatures.push_back(Json::object(std::move(sig)));
    sigHelp["signatures"] = Json::array(std::move(signatures));
    sigHelp["activeSignature"] = Json::number(0);
    sigHelp["activeParameter"] = Json::number(std::max(0, std::min(activeParam, std::max(0, ar - 1))));

    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::object(std::move(sigHelp));
    writeMessage(Json::object(std::move(resp)));
}

static void handleRename(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    auto pPos = getPath(req, {"params", "position"});
    auto pNew = getPath(req, {"params", "newName"});
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    int line = pPos ? static_cast<int>((**pPos).o.at("line").n) : 0;
    int ch = pPos ? static_cast<int>((**pPos).o.at("character").n) : 0;
    std::string newName = pNew ? (**pNew).s : "";
    std::string targetName;
    auto it = g_docs.find(uri);
    if (it != g_docs.end()) {
        for (const auto& [name, sym] : it->second.symbols) {
            if (line == sym.def.start.line && ch >= sym.def.start.character && ch <= sym.def.end.character) {
                targetName = name;
                break;
            }
        }
        if (targetName.empty()) {
            for (const auto& t : it->second.tokens) {
                if (t.line - 1 == line) {
                    int start = t.column - 1;
                    int end = start + static_cast<int>(t.lexeme.size());
                    if (ch >= start && ch <= end && t.type == TokenType::Identifier) {
                        targetName = std::string(t.lexeme);
                        break;
                    }
                }
            }
        }
    }
    JsonObject edit;
    JsonObject changes;
    if (!targetName.empty() && !newName.empty()) {
        auto addDocEdits = [&](Document& doc) {
            JsonArray edits;
            auto sit = doc.symbols.find(targetName);
            if (sit != doc.symbols.end()) {
                JsonObject te;
                te["range"] = makeRange(sit->second.def);
                te["newText"] = Json::string(newName);
                edits.push_back(Json::object(std::move(te)));
                for (const auto& rr : sit->second.refs) {
                    JsonObject te2;
                    te2["range"] = makeRange(rr);
                    te2["newText"] = Json::string(newName);
                    edits.push_back(Json::object(std::move(te2)));
                }
            }
            auto rit = doc.refByName.find(targetName);
            if (rit != doc.refByName.end()) {
                for (const auto& rr : rit->second) {
                    JsonObject te2;
                    te2["range"] = makeRange(rr);
                    te2["newText"] = Json::string(newName);
                    edits.push_back(Json::object(std::move(te2)));
                }
            }
            if (!edits.empty()) {
                changes[doc.uri] = Json::array(std::move(edits));
            }
        };
        for (auto& [du, d] : g_docs) addDocEdits(d);
        for (auto& [path, d] : g_wsdocs) addDocEdits(d);
    }
    edit["changes"] = Json::object(std::move(changes));
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::object(std::move(edit));
    writeMessage(Json::object(std::move(resp)));
}

static void handleDidOpen(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    if (!pDoc) return;
    const Json& d = **pDoc;
    std::string uri = d.o.at("uri").s;
    std::string text = d.o.at("text").s;
    Document doc;
    doc.uri = uri;
    doc.path = uri;
    doc.text = text;
    analyzeDocument(doc);
    g_docs[uri] = std::move(doc);
    // Update workspace index entry too
    std::string path = uriToPath(uri);
    auto itws = g_wsdocs.find(path);
    if (itws != g_wsdocs.end()) {
        itws->second.text = d.o.at("text").s;
        itws->second.uri = uri;
        analyzeDocument(itws->second);
    } else {
        Document wd;
        wd.path = path;
        wd.uri = uri;
        wd.text = d.o.at("text").s;
        analyzeDocument(wd);
        g_wsdocs[path] = std::move(wd);
    }
    sendDiagnostics(g_docs[uri]);
}

static void handleDidChange(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    auto pCh = getPath(req, {"params", "contentChanges"});
    if (!pDoc || !pCh) return;
    const Json& d = **pDoc;
    std::string uri = d.o.at("uri").s;
    auto it = g_docs.find(uri);
    if (it == g_docs.end()) return;
    const Json& changes = **pCh;
    if (!changes.a.empty()) {
        it->second.text = changes.a[0].o.at("text").s;
        analyzeDocument(it->second);
        sendDiagnostics(it->second);
        std::string path = uriToPath(uri);
        auto itws = g_wsdocs.find(path);
        if (itws != g_wsdocs.end()) {
            itws->second.text = changes.a[0].o.at("text").s;
            itws->second.uri = uri;
            analyzeDocument(itws->second);
        } else {
            Document wd;
            wd.path = path;
            wd.uri = uri;
            wd.text = changes.a[0].o.at("text").s;
            analyzeDocument(wd);
            g_wsdocs[path] = std::move(wd);
        }
    }
}

static void handleHover(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    auto pPos = getPath(req, {"params", "position"});
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    JsonObject result;
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    int line = pPos ? static_cast<int>((**pPos).o.at("line").n) : 0;
    int ch = pPos ? static_cast<int>((**pPos).o.at("character").n) : 0;
    std::string typeName = "unknown";
    std::string signature;
    auto it = g_docs.find(uri);
    if (it != g_docs.end()) {
        for (const auto& [name, sym] : it->second.symbols) {
            if (line == sym.def.start.line &&
                ch >= sym.def.start.character && ch <= sym.def.end.character) {
                typeName = sym.typeName.empty() ? "symbol" : sym.typeName;
                if (sym.typeName == "function") {
                    signature = name + "(";
                    for (size_t i = 0; i < sym.params.size(); ++i) {
                        signature += sym.params[i];
                        if (i + 1 < sym.params.size()) signature += ", ";
                    }
                    signature += ")";
                }
                break;
            }
        }
        if (signature.empty()) {
            for (const auto& [lname, lsym] : it->second.locals) {
                if (line == lsym.def.start.line &&
                    ch >= lsym.def.start.character && ch <= lsym.def.end.character) {
                    typeName = lsym.typeName.empty() ? "symbol" : lsym.typeName;
                    break;
                }
            }
        }
        for (const auto& t : it->second.tokens) {
            if (t.line - 1 == line) {
                int start = t.column - 1;
                int end = start + static_cast<int>(t.lexeme.size());
                if (ch >= start && ch <= end) {
                    switch (t.type) {
                        case TokenType::Number: typeName = "number"; break;
                        case TokenType::String: typeName = "string"; break;
                        case TokenType::Identifier: break;
                        default: break;
                    }
                    break;
                }
            }
        }
    }
    JsonObject contents;
    contents["kind"] = Json::string("markdown");
    std::string val = "Type: " + typeName;
    if (!signature.empty()) val += std::string("\nSignature: ") + signature;
    contents["value"] = Json::string(val);
    result["contents"] = Json::object(std::move(contents));
    resp["result"] = Json::object(std::move(result));
    writeMessage(Json::object(std::move(resp)));
}

static void handleDefinition(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    auto pPos = getPath(req, {"params", "position"});
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    int line = pPos ? static_cast<int>((**pPos).o.at("line").n) : 0;
    int ch = pPos ? static_cast<int>((**pPos).o.at("character").n) : 0;
    JsonArray locations;
    auto it = g_docs.find(uri);
    std::string targetName;
    if (it != g_docs.end()) {
        for (const auto& [name, sym] : it->second.symbols) {
            for (const auto& ref : sym.refs) {
                if (line == ref.start.line && ch >= ref.start.character && ch <= ref.end.character) {
                    locations.push_back(makeLocation(uri, sym.def));
                    targetName = name;
                    break;
                }
            }
        }
        if (targetName.empty()) {
            for (const auto& t : it->second.tokens) {
                if (t.line - 1 == line) {
                    int start = t.column - 1;
                    int end = start + static_cast<int>(t.lexeme.size());
                    if (ch >= start && ch <= end && t.type == TokenType::Identifier) {
                        targetName = std::string(t.lexeme);
                        break;
                    }
                }
            }
        }
    }
    if (!targetName.empty()) {
        for (const auto& [du, doc] : g_docs) {
            auto sit = doc.symbols.find(targetName);
            if (sit != doc.symbols.end()) {
                locations.push_back(makeLocation(du, sit->second.def));
            }
        }
        for (const auto& [path, doc] : g_wsdocs) {
            auto sit = doc.symbols.find(targetName);
            if (sit != doc.symbols.end()) {
                locations.push_back(makeLocation(doc.uri, sit->second.def));
            }
        }
    }
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::array(std::move(locations));
    writeMessage(Json::object(std::move(resp)));
}

static void handleReferences(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    auto pPos = getPath(req, {"params", "position"});
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    int line = pPos ? static_cast<int>((**pPos).o.at("line").n) : 0;
    int ch = pPos ? static_cast<int>((**pPos).o.at("character").n) : 0;
    JsonArray locations;
    auto it = g_docs.find(uri);
    std::string targetName;
    if (it != g_docs.end()) {
        for (const auto& [name, sym] : it->second.symbols) {
            if (line == sym.def.start.line && ch >= sym.def.start.character && ch <= sym.def.end.character) {
                targetName = name;
                for (const auto& ref : sym.refs) {
                    locations.push_back(makeLocation(uri, ref));
                }
                break;
            }
        }
        if (targetName.empty()) {
            for (const auto& t : it->second.tokens) {
                if (t.line - 1 == line) {
                    int start = t.column - 1;
                    int end = start + static_cast<int>(t.lexeme.size());
                    if (ch >= start && ch <= end && t.type == TokenType::Identifier) {
                        targetName = std::string(t.lexeme);
                        break;
                    }
                }
            }
            if (!targetName.empty()) {
                auto sit = it->second.symbols.find(targetName);
                if (sit != it->second.symbols.end()) {
                    for (const auto& ref : sit->second.refs) {
                        locations.push_back(makeLocation(uri, ref));
                    }
                }
            }
        }
    }
    if (!targetName.empty()) {
        for (const auto& [du, doc] : g_docs) {
            auto rit = doc.refByName.find(targetName);
            if (rit != doc.refByName.end()) {
                for (const auto& rr : rit->second) {
                    locations.push_back(makeLocation(du, rr));
                }
            }
        }
        for (const auto& [path, doc] : g_wsdocs) {
            auto rit = doc.refByName.find(targetName);
            if (rit != doc.refByName.end()) {
                for (const auto& rr : rit->second) {
                    locations.push_back(makeLocation(doc.uri, rr));
                }
            }
        }
    }
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::array(std::move(locations));
    writeMessage(Json::object(std::move(resp)));
}

static void handleCompletion(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    auto pPos = getPath(req, {"params", "position"});
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    JsonArray items;
    auto it = g_docs.find(uri);
    if (it != g_docs.end()) {
        int line = pPos ? static_cast<int>((**pPos).o.at("line").n) : 0;
        int ch = pPos ? static_cast<int>((**pPos).o.at("character").n) : 0;
        std::istringstream iss(it->second.text);
        std::string curLine;
        int cur = 0;
        while (std::getline(iss, curLine)) {
            if (!curLine.empty() && curLine.back() == '\r') curLine.pop_back();
            if (cur == line) break;
            cur++;
        }
        bool afterDot = false;
        if (ch > 0 && ch <= static_cast<int>(curLine.size())) {
            for (int i = ch - 1; i >= 0; --i) {
                char c = curLine[i];
                if (c == '.') { afterDot = true; break; }
                if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) break;
            }
        }
        if (afterDot) {
            const char* arrayMethods[] = {"push","pop","reverse","map","filter","reduce","join","concat","slice","flat","flatMap","length"};
            for (const char* m : arrayMethods) {
                JsonObject item;
                item["label"] = Json::string(m);
                item["kind"] = Json::number(2);
                items.push_back(Json::object(std::move(item)));
            }
            const char* hashmapMethods[] = {"keys","values","has","remove","size"};
            for (const char* m : hashmapMethods) {
                JsonObject item;
                item["label"] = Json::string(m);
                item["kind"] = Json::number(2);
                items.push_back(Json::object(std::move(item)));
            }
        }
        const char* keywords[] = {"let","fn","class","init","return","if","else","while","for","run","until","break","continue","try","catch","throw"};
        for (const char* kw : keywords) {
            JsonObject item;
            item["label"] = Json::string(kw);
            item["kind"] = Json::number(14);
            items.push_back(Json::object(std::move(item)));
        }
        const char* builtins[] = {
            "len","str","substr","toUpper","toLower","split","trim","indexOf",
            "pow","sqrt","sin","cos","tan","abs","min","max","round","floor","ceil","random",
            "readFile","writeFile","appendFile","exists","fileSize",
            "keys","values","has","remove",
            "compose","pipe","benchmark","sleep","now","formatDate",
            "jsonEncode","jsonDecode","type"
        };
        for (const char* bn : builtins) {
            JsonObject item;
            item["label"] = Json::string(bn);
            item["kind"] = Json::number(3);
            items.push_back(Json::object(std::move(item)));
        }
        for (const auto& [name, sym] : it->second.symbols) {
            JsonObject item;
            item["label"] = Json::string(name);
            item["kind"] = Json::number(sym.typeName == "function" ? 3 : 6);
            if (sym.typeName == "function") {
                std::string sig = name + "(";
                for (size_t i = 0; i < sym.params.size(); ++i) {
                    sig += sym.params[i];
                    if (i + 1 < sym.params.size()) sig += ", ";
                }
                sig += ")";
                item["detail"] = Json::string(sig);
                std::ostringstream snippet;
                snippet << name << "(";
                for (size_t i = 0; i < sym.params.size(); ++i) {
                    snippet << "${" << (i + 1) << ":" << sym.params[i] << "}";
                    if (i + 1 < sym.params.size()) snippet << ", ";
                }
                snippet << ")";
                item["insertTextFormat"] = Json::number(2);
                item["insertText"] = Json::string(snippet.str());
            }
            items.push_back(Json::object(std::move(item)));
        }
        for (const auto& [lname, lsym] : it->second.locals) {
            JsonObject item;
            item["label"] = Json::string(lname);
            item["kind"] = Json::number(6);
            items.push_back(Json::object(std::move(item)));
        }
    }
    if (g_interp) {
        auto globals = g_interp->getGlobals();
        globals->forEachKey([&](std::string_view sv) {
            JsonObject item;
            item["label"] = Json::string(std::string(sv));
            Value v = globals->get(sv);
            if (isCallable(v)) {
                auto fn = asCallable(v);
                int ar = fn ? fn->arity() : 0;
                item["kind"] = Json::number(3);
                std::string sig = std::string(sv) + "(";
                for (int i = 0; i < ar; ++i) {
                    sig += "arg" + std::to_string(i + 1);
                    if (i + 1 < ar) sig += ", ";
                }
                sig += ")";
                item["detail"] = Json::string(sig);
                std::ostringstream snippet;
                snippet << std::string(sv) << "(";
                for (int i = 0; i < ar; ++i) {
                    snippet << "${" << (i + 1) << ":arg" << (i + 1) << "}";
                    if (i + 1 < ar) snippet << ", ";
                }
                snippet << ")";
                item["insertTextFormat"] = Json::number(2);
                item["insertText"] = Json::string(snippet.str());
            } else {
                item["kind"] = Json::number(6);
            }
            items.push_back(Json::object(std::move(item)));
        });
    }
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::array(std::move(items));
    writeMessage(Json::object(std::move(resp)));
}

static void handleRangeFormatting(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    auto pRange = getPath(req, {"params", "range"});
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    JsonArray edits;
    auto it = g_docs.find(uri);
    if (it != g_docs.end() && pRange) {
        int sLine = static_cast<int>((**pRange).o.at("start").o.at("line").n);
        int sChar = static_cast<int>((**pRange).o.at("start").o.at("character").n);
        int eLine = static_cast<int>((**pRange).o.at("end").o.at("line").n);
        int eChar = static_cast<int>((**pRange).o.at("end").o.at("character").n);
        std::istringstream iss(it->second.text);
        std::string line;
        std::vector<std::string> lines;
        while (std::getline(iss, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            lines.push_back(line);
        }
        std::ostringstream sub;
        for (int i = sLine; i <= eLine && i < static_cast<int>(lines.size()); ++i) {
            if (i == sLine && i == eLine) {
                sub << lines[i].substr(sChar, std::max(0, eChar - sChar));
            } else if (i == sLine) {
                sub << lines[i].substr(sChar) << "\n";
            } else if (i == eLine) {
                sub << lines[i].substr(0, std::min(eChar, static_cast<int>(lines[i].size())));
            } else {
                sub << lines[i] << "\n";
            }
        }
        Lexer lex(sub.str());
        auto toks = lex.tokenize();
        std::string formatted = claw::fmt::formatTokens(toks, 2);
        JsonObject te;
        Range rr; rr.start.line = sLine; rr.start.character = sChar; rr.end.line = eLine; rr.end.character = eChar;
        te["range"] = makeRange(rr);
        te["newText"] = Json::string(formatted);
        edits.push_back(Json::object(std::move(te)));
    }
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::array(std::move(edits));
    writeMessage(Json::object(std::move(resp)));
}

static void handleCodeAction(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    auto pRange = getPath(req, {"params", "range"});
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    Range rr;
    if (pRange) {
        rr.start.line = static_cast<int>((**pRange).o.at("start").o.at("line").n);
        rr.start.character = static_cast<int>((**pRange).o.at("start").o.at("character").n);
        rr.end.line = static_cast<int>((**pRange).o.at("end").o.at("line").n);
        rr.end.character = static_cast<int>((**pRange).o.at("end").o.at("character").n);
    } else {
        rr.start.line = 0; rr.start.character = 0; rr.end.line = INT32_MAX; rr.end.character = INT32_MAX;
    }
    JsonArray actions;
    auto it = g_docs.find(uri);
    if (it != g_docs.end()) {
        for (const auto& si : it->second.styleIssues) {
            const Range& r = si.first;
            bool overlaps = !(r.end.line < rr.start.line || r.start.line > rr.end.line);
            if (!overlaps) continue;
            JsonObject edit;
            JsonArray edits;
            JsonObject te;
            te["range"] = makeRange(r);
            te["newText"] = Json::string("");
            edits.push_back(Json::object(std::move(te)));
            JsonObject changes;
            changes[it->second.uri] = Json::array(std::move(edits));
            edit["changes"] = Json::object(std::move(changes));
            JsonObject act;
            act["title"] = Json::string(si.second);
            act["kind"] = Json::string("quickfix");
            act["edit"] = Json::object(std::move(edit));
            actions.push_back(Json::object(std::move(act)));
        }
    }
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::array(std::move(actions));
    writeMessage(Json::object(std::move(resp)));
}
static void handleOnTypeFormatting(const Json& req) {
    auto pDoc = getPath(req, {"params", "textDocument"});
    auto pPos = getPath(req, {"params", "position"});
    std::string uri = pDoc ? (**pDoc).o.at("uri").s : "";
    int line = pPos ? static_cast<int>((**pPos).o.at("line").n) : 0;
    JsonArray edits;
    auto it = g_docs.find(uri);
    if (it != g_docs.end()) {
        std::istringstream iss(it->second.text);
        std::string textLine;
        int cur = 0;
        while (std::getline(iss, textLine)) {
            if (!textLine.empty() && textLine.back() == '\r') textLine.pop_back();
            if (cur == line) break;
            cur++;
        }
        Lexer lex(textLine);
        auto toks = lex.tokenize();
        std::string formatted = claw::fmt::formatTokens(toks, 2);
        Range rr; rr.start.line = line; rr.start.character = 0; rr.end.line = line; rr.end.character = static_cast<int>(textLine.size());
        JsonObject te;
        te["range"] = makeRange(rr);
        te["newText"] = Json::string(formatted);
        edits.push_back(Json::object(std::move(te)));
    }
    JsonObject resp;
    resp["jsonrpc"] = Json::string("2.0");
    auto idOpt = getPath(req, {"id"});
    if (idOpt) resp["id"] = *(idOpt.value());
    resp["result"] = Json::array(std::move(edits));
    writeMessage(Json::object(std::move(resp)));
}
int run() {
    while (true) {
        auto msg = readMessage();
        if (!msg) break;
        try {
            JsonParser parser(*msg);
            Json req = parser.parse();
            auto m = getPath(req, {"method"});
            if (m && (**m).type == Json::Type::String) {
                std::string method = (**m).s;
                if (method == "initialize") { handleInitialize(req); continue; }
                if (method == "textDocument/didOpen") { handleDidOpen(req); continue; }
                if (method == "textDocument/didChange") { handleDidChange(req); continue; }
                if (method == "textDocument/hover") { handleHover(req); continue; }
                if (method == "textDocument/definition") { handleDefinition(req); continue; }
                if (method == "textDocument/references") { handleReferences(req); continue; }
                if (method == "textDocument/completion") { handleCompletion(req); continue; }
                if (method == "textDocument/documentSymbol") { handleDocumentSymbol(req); continue; }
                if (method == "textDocument/signatureHelp") { handleSignatureHelp(req); continue; }
                if (method == "workspace/symbol") { handleWorkspaceSymbol(req); continue; }
                if (method == "textDocument/rename") { handleRename(req); continue; }
                if (method == "textDocument/formatting") { handleFormatting(req); continue; }
                if (method == "textDocument/rangeFormatting") { handleRangeFormatting(req); continue; }
                if (method == "textDocument/onTypeFormatting") { handleOnTypeFormatting(req); continue; }
                if (method == "textDocument/codeAction") { handleCodeAction(req); continue; }
            } else {
                auto idOpt = getPath(req, {"id"});
                if (idOpt) {
                    JsonObject resp;
                    resp["jsonrpc"] = Json::string("2.0");
                    resp["id"] = *(idOpt.value());
                    resp["result"] = Json::null();
                    writeMessage(Json::object(std::move(resp)));
                }
            }
        } catch (...) {
        }
    }
    return 0;
}

} // namespace claw::lsp

int main() { return claw::lsp::run(); }
