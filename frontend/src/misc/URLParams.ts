import Point from "./Point";
import { complexToLeaflet, leafletToComplex } from "../tileDisplay/Project";

export const getURLParams = () => {
  const params = new URLSearchParams(document.location.search);
  const real = parseFloat(params.get("real") as string);
  const imag = parseFloat(params.get("imag") as string);
  const zoom = parseFloat(params.get("zoom") as string);
  if (!real || !imag || !zoom) {
    return new Point(0, 0, 0);
  }
  return complexToLeaflet(real, imag, zoom);
};

export const setURLParams = (point: Point) => {
  const complex = leafletToComplex(point.x, point.y, point.z);
  const params = new URLSearchParams(document.location.search);
  params.set("real", complex.x.toString());
  params.set("imag", complex.y.toString());
  params.set("zoom", complex.z.toString());
  // replace old url params
  window.history.replaceState({}, "", `${document.location.pathname}?${params}`);
  return complex;
};
