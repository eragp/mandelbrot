import Point from "./Point";
import { complexToLeaflet, leafletToComplex } from "../tileDisplay/Project";

export const getURLParams = () => {
  let params = new URLSearchParams(document.location.search);
  let real = parseFloat(params.get("real")),
    imag = parseFloat(params.get("imag")),
    zoom = parseFloat(params.get("zoom"));
  if (real === undefined || imag === undefined || zoom === undefined) {
    return new Point(0, 0, 3);
  }
  return complexToLeaflet(real, imag, zoom);
};

export const setURLParams = point => {
  console.log(`input: ${point}`);
  point = leafletToComplex(point.x, point.y, point.z);
  console.log(`output: ${point}`);

  let params = new URLSearchParams(document.location.search);
  params.set("real", point.x);
  params.set("imag", point.y);
  params.set("zoom", point.z);
  // replace old url params
  window.history.replaceState(
    {},
    "",
    `${document.location.pathname}?${params}`
  );
};
