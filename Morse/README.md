## Morse Code
Creates a morse code of a given word. If running on **x86_64** it will print `Dot ` or `Dash ` into the standard output. If running on **arm** processor(BeagleBone Black) it will blink the third LED on board.

### How to build and run x86_64 version
```
make 
cd Build/
./Mcode -w Welcome
```

### How to build and run arm version
```
make arm=1
cd Build/
```
Once the Mcode it built, copy to the target and run `./Mcode -w Hello`
