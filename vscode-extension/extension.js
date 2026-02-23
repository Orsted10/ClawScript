const cp = require('child_process');
const net = require('net');
const path = require('path');
const vscode = require('vscode');

function activate(context) {
  const serverExe = path.join(vscode.workspace.rootPath || "", "build", "bin", "Release", "claw_lsp.exe");
  const altExe = path.join(vscode.workspace.rootPath || "", "build", "bin", "Debug", "claw_lsp.exe");
  const executable = require('fs').existsSync(serverExe) ? serverExe : altExe;
  const serverOptions = {
    command: executable,
    args: [],
    options: {}
  };
  const clientOptions = {
    documentSelector: [{ scheme: 'file', language: 'claw' }],
    synchronize: {
      fileEvents: vscode.workspace.createFileSystemWatcher('**/*.claw')
    }
  };
  const client = new (require('vscode-languageclient').LanguageClient)(
    'clawLanguageServer', 'Claw LSP', serverOptions, clientOptions
  );
  context.subscriptions.push(client.start());

  const fmtExe = path.join(vscode.workspace.rootPath || "", "build", "bin", "Release", "claw-fmt.exe");
  const fmtAlt = path.join(vscode.workspace.rootPath || "", "build", "bin", "Debug", "claw-fmt.exe");
  const formatter = require('fs').existsSync(fmtExe) ? fmtExe : fmtAlt;
  function computeEdits(document) {
    const tmp = require('os').tmpdir();
    const fs = require('fs');
    const tmpFile = path.join(tmp, 'claw_fmt_tmp.claw');
    fs.writeFileSync(tmpFile, document.getText(), 'utf8');
    try {
      cp.execFileSync(formatter, ['--write', `--root=${tmp}`], {stdio:'pipe'});
      const formatted = fs.readFileSync(tmpFile, 'utf8');
      const fullRange = new vscode.Range(0, 0, document.lineCount, 0);
      return [vscode.TextEdit.replace(fullRange, formatted)];
    } catch (e) {
      return [];
    } finally {
      try { require('fs').unlinkSync(tmpFile); } catch {}
    }
  }
  vscode.languages.registerDocumentFormattingEditProvider('claw', {
    provideDocumentFormattingEdits(document) { return computeEdits(document); }
  });
  context.subscriptions.push(vscode.workspace.onWillSaveTextDocument(e => {
    if (e.document.languageId !== 'claw') return;
    e.waitUntil(Promise.resolve(computeEdits(e.document)));
  }));
}

function deactivate() {}

module.exports = { activate, deactivate };
