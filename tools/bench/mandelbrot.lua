local function mandelbrot(w, h, maxIter)
  local count = 0
  for i = 0, h - 1 do
    for j = 0, w - 1 do
      local x = (j / (w / 2) - 1.5)
      local y = (i / (h / 2) - 1.0)
      local a = 0.0
      local b = 0.0
      local k = 0
      while k < maxIter do
        local aa = a * a - b * b + x
        local bb = 2 * a * b + y
        a = aa
        b = bb
        if a * a + b * b > 4.0 then
          break
        end
        k = k + 1
      end
      if k == maxIter then
        count = count + 1
      end
    end
  end
  return count
end

print(mandelbrot(80, 40, 100))
