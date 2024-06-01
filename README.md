# bb
Boring Booking, a demo dockerized C++ project managed by Conan.

## To build and run on your own system
```sh
git clone https://github.com/leixure/bb.git
cd bb
conan create . --build=missing
```
expect something like this:
```console
...
======== Testing the package: Executing test ========
bb/1.0 (test package): Running test()
bb/1.0 (test package): RUN: bb
Navigate to http://localhost:8080
```
Then open the link above in your browser.

## To examine the artifacts generated
```sh
# assuming you are in the project root, and you have not run `conan create`
conan create . --build=missing -tf ""

# then
conan install --requires bb/1.0 --deployer direct_deployer
cd direct_deployer/bb
```

## To build and run the docker image
```sh
# build the docker image locally
docker buildx build -t lxiong/bb .
# run the image
docker run -p 8080:8080 lxiong/bb:latest
```

## To run the image available on docker hub
```sh
docker pull lxiong/bb
docker run -p 8080:8080 lxiong/bb:latest
```
