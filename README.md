# digital-piano

A mixer, sampler and keyboard interface written for raspberry pi in C. The keyboard should be a full 88 key dual sensor keyboard. noteListen.c holds the main() function which runs the sampler and scans the keys. noteFunc.c holds extra functions that the main() function uses.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

Note that this library will have to be modified to get it working with different types of keyboards. Minimal modification should be necessary however.

### Prerequisites

Required libraries:

```
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pigpio.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include "noteListen.h"

```

## Authors

* **Kevin McAndrew** - *Student of Electronic and computer engineering (University of Limerick)*

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Eoin McArdle designed the case for this piano. He is a computer science student currently studying in the National University of Ireland Galway.
