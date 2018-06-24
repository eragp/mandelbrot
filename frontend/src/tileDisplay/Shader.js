class Shader {
  static default(n, maxIteration) {
    let r = (n * 10) % 256,
      g = (n * 20) % 256,
      b = (n * 40) % 256;
    return [r, g, b];
  }

  static testPattern(x, y) {
    return [x ^ y, x ^ y, x ^ y];
  }
}
export default Shader;
