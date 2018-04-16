FROM maddinat0r/debian-samp

RUN apt-get install libcpprest-dev
ADD . .
RUN mkdir build
ENTRYPOINT [ "make", "build-inside" ]