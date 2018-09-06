import Point from "./Point";
import { complexToLeaflet, leafletToComplex } from "../tileDisplay/Project";

export const getURLParams = () => {
  let params = new URLSearchParams(document.location.search);
  let real = parseFloat(params.get("real")),
    imag = parseFloat(params.get("imag")),
    zoom = parseFloat(params.get("zoom"));
  if (!real || !imag || !zoom) {
    return new Point(0, 0, 0);
  }
  return complexToLeaflet(real, imag, zoom);
};

export const setURLParams = point => {
  let complex = leafletToComplex(point.x, point.y, point.z);

  let params = new URLSearchParams(document.location.search);
  params.set("real", complex.x);
  params.set("imag", complex.y);
  params.set("zoom", complex.z);
  // replace old url params
  window.history.replaceState({}, "", `${document.location.pathname}?${params}`);
  return complex;
};
