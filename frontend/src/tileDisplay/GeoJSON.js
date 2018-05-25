const size = 20;
const maxWidth = 400;
const maxHeight = 300;

const generateJSON = () => {
  let id = 0;
  let currHeight = 0;
  let features = [];
  while (currHeight < maxHeight) {
    let yMin = currHeight;
    let yMax = (currHeight += size);
    let currWidth = 0;
    while (currWidth < maxWidth) {
      let xMin = currWidth;
      let xMax = (currWidth += size);
      features.push({
        type: 'Feature',
        id: (id++).toString(),
        properties: {
          node: 'Node-' + randomNumber(0, 10),
          time: randomNumber(0, 1000)
        },
        geometry: {
          type: 'Polygon',
          coordinates: [
            [
              [xMin, yMin],
              [xMax, yMin],
              [xMax, yMax],
              [xMin, yMax],
              [xMin, yMin]
            ]
          ]
        }
      });
    }
  }
  let obj = {
    type: 'FeatureCollection',
    features: features
  };
  return obj;
};

const randomNumber = (min, max) => {
  return Math.floor(Math.random() * max) + min;
};
export default generateJSON;
