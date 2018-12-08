interface Point {
  toString(): string;
  equals(otherPoint: Point): boolean;
}

/**
 * Models a 3D Point
 */
export class Point3D implements Point {
  public x: number;
  public y: number;
  public z: number;

  constructor(x = 0, y = 0, z = 0) {
    this.x = x;
    this.y = y;
    this.z = z;
  }

  public toString(): string {
    return `Point3D{${this.x}, ${this.y}, ${this.z}}`;
  }

  public equals(otherPoint: Point): boolean {
    if (!otherPoint) {
      return false;
    }
    let pt = <Point3D>otherPoint;
    const aProps = Object.getOwnPropertyNames(this);
    const bProps = Object.getOwnPropertyNames(pt);

    if (aProps.length !== bProps.length) {
      return false;
    }
    let dx = this.x - pt.x,
      dy = this.y - pt.y,
      dz = this.z - pt.z;
    return dx < Number.EPSILON && dy < Number.EPSILON && dz < Number.EPSILON;
  }
}

/**
 * Models a 2D Point
 */
export class Point2D implements Point {
  public x: number;
  public y: number;

  constructor(x = 0, y = 0) {
    this.x = x;
    this.y = y;
  }

  public toString(): string {
    return `Point2D{${this.x}, ${this.y}}`;
  }

  public equals(otherPoint: Point): boolean {
    if (!otherPoint) {
      return false;
    }
    let pt = <Point2D>otherPoint;
    const aProps = Object.getOwnPropertyNames(this);
    const bProps = Object.getOwnPropertyNames(pt);

    if (aProps.length !== bProps.length) {
      return false;
    }
    let dx = this.x - pt.x,
      dy = this.y - pt.y;
    return dx < Number.EPSILON && dy < Number.EPSILON;
  }
}
