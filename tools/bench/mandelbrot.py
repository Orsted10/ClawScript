def mandelbrot(w, h, max_iter):
    count = 0
    for i in range(h):
        for j in range(w):
            x = (j / (w / 2) - 1.5)
            y = (i / (h / 2) - 1.0)
            a = 0.0
            b = 0.0
            k = 0
            while k < max_iter:
                aa = a * a - b * b + x
                bb = 2 * a * b + y
                a = aa
                b = bb
                if a * a + b * b > 4.0:
                    break
                k += 1
            if k == max_iter:
                count += 1
    return count

if __name__ == "__main__":
    print(mandelbrot(80, 40, 100))
