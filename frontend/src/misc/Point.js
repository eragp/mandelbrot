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
      "Point{" +
      this.x.toString() +
      ", " +
      this.y.toString() +
      ", " +
      this.z.toString() +
      "}"
    );
  }

  equals(otherPoint) {
    if (otherPoint === null) {
      return false;
    }
    let aProps = Object.getOwnPropertyNames(this);
    let bProps = Object.getOwnPropertyNames(otherPoint);

    if (aProps.length !== bProps.length) {
      return false;
    }
    for (let i = 0; i < aProps.length; i++) {
      let aName = aProps[i],
        bName = bProps[i];
      if (aName !== bName || this[aName] !== otherPoint[bName]) {
        return false;
      }
    }
    return true;
  }
}

export default Point;
