const path = require("path");

const babelLoader = {
  test: /\.js$/,
  loader: "babel-loader",
  include: [path.resolve(__dirname, "../app")],
  query: { presets: ["es2017"] }
};

const cssLoader = {
  test: /\.css$/,
  use: ["style-loader", "css-loader"]
};

const fileLoader = {
  test: /\.(png|svg|jpg|gif)$/,
  use: ["file-loader"]
};

const htmlLoader = {
  test: /\.html$/,
  use: ["html-loader"]
};

module.exports = {
  entry: "./src/index.js",
  output: {
    filename: "bundle.js",
    path: path.resolve(__dirname, "public")
  },
  module: {
    rules: [cssLoader, fileLoader, htmlLoader]
  }
};
