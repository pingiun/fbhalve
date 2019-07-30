# fbhalve
`fbhalve` converts unicode art files that use U+2588 FULL BLOCK characters to
an image that is halve the size of the original, by using smaller unicode
block characters such as U+2580 UPPER HALF BLOCK.

## Demonstration
[![asciicast](https://asciinema.org/a/K3FE4Wc62IiUvuGirPqc2Cor4.svg)](https://asciinema.org/a/K3FE4Wc62IiUvuGirPqc2Cor4)

Keep in mind that asciinema does not show FULL BLOCK characters that well, 
normally the characters "connect" in a terminal.

## Building

You need any C compiler to build fbhalve. Builds can be done using the
make command:
    
    $ make

## Contributions

[Send patches](https://git-send-email.io) and questions to
[~pingiun/fbhalve](~pingiun/fbhalve@lists.sr.ht).

Bugs & todo here: [~pingiun/fbhalve](https://todo.sr.ht/~pingiun/fbhalve)
