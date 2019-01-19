class Shader {
  public static simple(n: number, maxIteration: number): number[] {
    return [10, 20, 40].map(m => (n * m) % 256);
  }

  // Copied from https://giansass.com/blog/mandelbrot-set-part-2-opengl-program/
  public static smooth2(n: number, maxIteration: number): number[] {
    const t = n / maxIteration;
    return [
      9 * (1 - t) * t * t * t,
      15 * (1 - t) * (1 - t) * t * t,
      8.5 * (1 - t) * (1 - t) * (1 - t) * t
    ].map(v => (v * 255) % 256);
  }

  /**
   * The smooth color mapping linearly interpolates the colors given by Division Colors
   * @param n iteration
   * @param maxIteration  max iteration
   */
  public static smooth(n: number, maxIteration: number): number[] {
    // Black if in the mandelbrot set
    if (n === maxIteration) {
      return [0, 0, 0];
    }

    const divisionPoints: number[] = [0, 0.05, 0.15, 0.33, 0.67, 0.85, 1];
    const divisionColors: number[][] = [
      //
      [1, 1, 1],
      [0.3, 0.3, 1],
      [0.5, 0, 0.7],
      [0.9, 0.5, 0.3],
      [1, 0.6, 0.1],
      [1, 0.7, 0],
      [1, 1, 1]
    ];

    const position = n / maxIteration; // 0.0 <= position <= 1.0
    // ====================================================
    // Taken from view-source:http://math.hws.edu/eck/js/mandelbrot/MB.html ll. 1200-1255, adapted, transformed to TS
    const clamp = (x: number) => {
      x = 2 * (x / 2 - Math.floor(x / 2));
      if (x > 1) {
        x = 2 - x;
      }
      return x;
    };

    let pt = 1;
    while (position > divisionPoints[pt]) {
      pt++;
    }
    const ratio =
      (position - divisionPoints[pt - 1]) / (divisionPoints[pt] - divisionPoints[pt - 1]);
    const c1 = divisionColors[pt - 1];
    const c2 = divisionColors[pt];
    const a = c1[0] + ratio * (c2[0] - c1[0]);
    const b = c1[1] + ratio * (c2[1] - c1[1]);
    const c = c1[2] + ratio * (c2[2] - c1[2]);

    return [a, b, c].map(v => Math.round(clamp(v) * 255));
  }

  public static testPattern(x: number, y: number): number[] {
    return [x ^ y, x ^ y, x ^ y];
  }
}
export default Shader;
