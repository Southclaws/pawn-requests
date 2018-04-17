FROM maddinat0r/debian-samp

RUN apt update && apt install -y libcpprest-dev
ADD . .
RUN mkdir build
ENTRYPOINT [ "make", "build-inside" ]