class Shader {
  public static default(n: number, maxIteration: number): number[] {
    const r = (n * 10) % 256;
    const g = (n * 20) % 256;
    const b = (n * 40) % 256;
    return [r, g, b];
  }

  public static testPattern(x: number, y: number): number[] {
    return [x ^ y, x ^ y, x ^ y];
  }
}
export default Shader;
