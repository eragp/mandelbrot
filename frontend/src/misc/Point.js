/**
 * Models a 3D Point
 */
class Point {
  constructor(x, y, z) {
    this.x = x || 0;
    this.y = y || 0;
    this.z = z || 0;
  }
  toString() {
    return (
      'Point{' +
      this.x.toString() +
      ', ' +
      this.y.toString() +
      ', ' +
      this.z.toString() +
      '}'
    );
  }
}

export default Point;