/**
 * Models a 3D Point
 */
export default class Point {
  public x: number;
  public y: number;
  public z: number;

  constructor(x = 0, y = 0, z = 0) {
    this.x = x;
    this.y = y;
    this.z = z;
  }

  toString(): string {
    return `Point{${this.x}, ${this.y}, ${this.z}}`;
  }

  equals(otherPoint: Point): boolean {
    if (otherPoint === null) {
      return false;
    }
    let aProps = Object.getOwnPropertyNames(this);
    let bProps = Object.getOwnPropertyNames(otherPoint);

    if (aProps.length !== bProps.length) {
      return false;
    }
    return this.x === otherPoint.x && this.y === otherPoint.y && this.z === otherPoint.z;
  }
}
