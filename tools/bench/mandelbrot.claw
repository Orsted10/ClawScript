fn mandelbrot(w, h, maxIter) {
  let count = 0;
  for (let i = 0; i < h; i = i + 1) {
    for (let j = 0; j < w; j = j + 1) {
      let x = (j / (w / 2) - 1.5);
      let y = (i / (h / 2) - 1.0);
      let a = 0; let b = 0; let k = 0;
      while (k < maxIter) {
        let aa = a * a - b * b + x;
        let bb = 2 * a * b + y;
        a = aa; b = bb;
        if (a * a + b * b > 4) { break; }
        k = k + 1;
      }
      if (k == maxIter) { count = count + 1; }
    }
  }
  return count;
}

let res = mandelbrot(80, 40, 100);
print res;
