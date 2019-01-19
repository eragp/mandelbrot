import { Point3D } from "./Point";

export const readViewCenterParams = () => {
  const params = new URLSearchParams(document.location.search);
  const real = parseFloat(params.get("real") as string);
  const imag = parseFloat(params.get("imag") as string);
  const zoom = parseFloat(params.get("zoom") as string);
  if (!real || !imag || !zoom) {
    return new Point3D(0, 0, 0);
  }
  return new Point3D(real, imag, zoom);
};

export const readWSUrl = () =>
  new URLSearchParams(document.location.search).get("backend") as string;

export const setWSUrl = (url: string) => {
  const params = new URLSearchParams(document.location.search);
  params.set("backend", url);
  // replace old url params
  window.history.replaceState({}, "", `${document.location.pathname}?${params}`);
};
/**
 *
 * @param point in complex coordinates
 */
export const setViewCenterParams = (point: Point3D) => {
  const params = new URLSearchParams(document.location.search);
  params.set("real", point.x.toString());
  params.set("imag", point.y.toString());
  params.set("zoom", point.z.toString());
  // replace old url params
  window.history.replaceState({}, "", `${document.location.pathname}?${params}`);
  return point;
};
