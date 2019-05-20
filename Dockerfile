FROM ubuntu:18.10
MAINTAINER mitrkos
USER root

RUN apt-get -y update && apt-get install -y wget cmake g++ libevent-dev

ENV PROJECT_NAME server

WORKDIR /opt/$PROJECT_NAME/
ADD . /opt/$PROJECT_NAME/

RUN mkdir build &&\
    cd build &&\
    cmake -DCMAKE_BUILD_TYPE=Release .. &&\
    make

EXPOSE 80
EXPOSE 8080
CMD ["/opt/server/build/TP_http_server"]
