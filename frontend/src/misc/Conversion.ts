export const usToString = (t: number) => {
  const units = ["μs", "ms", "s", "min", "hr", "day", "week"];
  const divisors = [1000, 1000, 60, 60, 24, 7];
  let i = 0;
  while (i !== units.length && t > divisors[i]) {
    t = t / divisors[i];
    i++;
  }
  return `${(Math.round(t * 100) / 100).toFixed(2)} ${units[i]}`;
};
