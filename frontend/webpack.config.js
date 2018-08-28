// Based on template from https://github.com/chunliu/typescript-react-hot-reload
const { resolve } = require("path");
const webpack = require("webpack");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const OpenBrowserPlugin = require("open-browser-webpack-plugin");
const tsImportPluginFactory = require("ts-import-plugin");
const MiniCssExtractPlugin = require("mini-css-extract-plugin");

const DevServerPort = 3000;
module.exports = {
  mode: "development",
  context: resolve(__dirname, "src"),
  entry: [
    "webpack-dev-server/client?http://localhost:" + DevServerPort,
    // bundle the client for webpack-dev-server
    // and connect to the provided endpoint
    "webpack/hot/only-dev-server",
    // bundle the client for hot reloading
    // only- means to only hot reload for successful updates
    "./index.tsx"
    // the entry point of our app
  ],
  output: {
    filename: "mandelbrot.js",
    // the output bundle
    path: resolve(__dirname, "dist"),
    publicPath: "/"
    // necessary for HMR to know where to load the hot update chunks
  },
  devtool: "inline-source-map",
  resolve: {
    // Add '.ts' and '.tsx' as resolvable extensions.
    extensions: [".ts", ".tsx", ".js", "jsx", ".json"]
  },
  devServer: {
    // Change it if other port needs to be used
    port: DevServerPort.toString(),
    // enable HMR on the server
    hot: true,
    noInfo: true,
    quiet: false,
    // minimize the output to terminal.
    contentBase: resolve(__dirname, "src"),
    // match the output path
    publicPath: "/"
    // match the output `publicPath`
  },
  module: {
    rules: [
      {
        enforce: "pre",
        test: /\.(ts|tsx)?$/,
        loader: "tslint-loader",
        exclude: [resolve(__dirname, "node_modules")]
      },
      {
        test: /\.(ts|tsx)?$/,
        use: [
          {
            loader: "ts-loader",
            options: {
              transpileOnly: true,
              getCustomTransformers: () => ({
                before: [
                  tsImportPluginFactory({
                    libraryName: "antd",
                    libraryDirectory: "es",
                    style: "css"
                  })
                ]
              }),
              compilerOptions: {
                module: "es2015"
              }
            }
          }
        ],
        exclude: [resolve(__dirname, "node_modules")]
      },
      { enforce: "pre", test: /\.js$/, loader: "source-map-loader" },
      {
        test: /\.css$/,
        use: [MiniCssExtractPlugin.loader, "css-loader"]
      },
      {
        test: /\.less$/,
        use: [MiniCssExtractPlugin.loader, "css-loader", "less-loader"]
      },
      { test: /\.png$/, loader: "url-loader?limit=100000" },
      { test: /\.jpg$/, loader: "file-loader" },
      {
        test: /\.(woff|woff2)(\?v=\d+\.\d+\.\d+)?$/,
        loader: "url-loader?limit=10000&mimetype=application/font-woff"
      },
      {
        test: /\.ttf(\?v=\d+\.\d+\.\d+)?$/,
        loader: "url-loader?limit=10000&mimetype=application/octet-stream"
      },
      { test: /\.eot(\?v=\d+\.\d+\.\d+)?$/, loader: "file-loader" },
      {
        test: /\.svg(\?v=\d+\.\d+\.\d+)?$/,
        loader: "url-loader?limit=10000&mimetype=image/svg+xml"
      }
    ]
  },
  plugins: [
    new MiniCssExtractPlugin({
      filename: "style.css",
      chunkFilename: "[id].css"
    }),
    new webpack.HotModuleReplacementPlugin(),
    // enable HMR globally
    new webpack.NamedModulesPlugin(),
    // prints more readable module names in the browser console on HMR updates
    new HtmlWebpackPlugin({ template: resolve(__dirname, "public/index.html") }),
    // inject <script> in html file.
    new OpenBrowserPlugin({ url: "http://localhost:" + DevServerPort })
  ]
};
