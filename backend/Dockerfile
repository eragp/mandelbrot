# the gcc dockerfile uses the buildpack-deps package and is based on debian:stretch
FROM gcc:7
COPY . /usr/src/mandelbrot-backend
WORKDIR /usr/src/mandelbrot-backend
RUN g++ Mandelbrot.cpp main.cpp -o mandelbrot.out
# CMD ["./mandelbrot.out"]