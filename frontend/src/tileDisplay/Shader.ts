class Shader {
  public static default(n: number, maxIteration: number): number[] {
    // Black if in the mandelbrot set
    if (n == maxIteration)
      return [0,0,0];
  
    let divisionPoints: number[] = [0,0.05,0.15,0.33,0.67,0.85,1];
    let divisionColors: number[][] = [[1,1,1],[0.3,0.3,1],[0.5,0,0.7],[0.9,0.5,0.3],[1,0.6,0.1],[1,0.7,0],[1,1,1]];

    let position = n/maxIteration; // 0.0 <= position <= 1.0
    // ====================================================
    // Taken from view-source:http://math.hws.edu/eck/js/mandelbrot/MB.html ll. 1200-1255, adapted, transformed to TS
    let clamp = function (x:number) {
      x = 2*(x/2 - Math.floor(x/2));
      if (x > 1)
        x = 2 - x;
      return x;
    };

    let pt = 1;
    while (position > divisionPoints[pt])
        pt++;
    
    const ratio = (position - divisionPoints[pt-1]) /
                    (divisionPoints[pt] - divisionPoints[pt-1]);
    let c1 = divisionColors[pt-1];
    let c2 = divisionColors[pt];
    let a = c1[0] + ratio*(c2[0] - c1[0]);
    let b = c1[1] + ratio*(c2[1] - c1[1]);
    let c = c1[2] + ratio*(c2[2] - c1[2]);

    a = clamp(a);
    b = clamp(b);
    c = clamp(c);
    let color = [a,b,c];
    color[0] = Math.round(color[0]*255);
    color[1] = Math.round(color[1]*255);
    color[2] = Math.round(color[2]*255);
    return color;
    // ====================================================
  }

  public static testPattern(x: number, y: number): number[] {
    return [x ^ y, x ^ y, x ^ y];
  }
}
export default Shader;
