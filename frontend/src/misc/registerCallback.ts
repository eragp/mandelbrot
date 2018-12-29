/**
 * Registers an observer to a list
 * @param list list of callbacks
 * @param fun function to add to list
 * @returns Promise wrapping the return value of fun
 */
export const registerCallback = (list: Array<((data: any) => any)>, fun: (data: any) => any) => {
  return new Promise((resolve, error) => {
    try {
      list.push((data: any) => {
        const ret = fun(data);
        resolve(ret);
        return ret;
      });
    } catch (e) {
      error(e);
    }
  });
};
