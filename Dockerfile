FROM maddinat0r/debian-samp

ADD . .
RUN mkdir build
ENTRYPOINT [ "make", "build-inside" ]