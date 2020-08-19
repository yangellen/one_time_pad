# one_time_pad

Three small programs was created that encrypt and decrypt a basic text file using a one-time pad-like system. 

## keygen.c

This program creates a key file of specified length. The characters in the file generated are 26 capital letters, and the space character().

syntax for keygen
    keygen keyLength

keyLength: the length of the kiey file in characters


## otp_d.c

This program run in the background as a daemon. The program functions to store the encrypted data and support up to five concurrent socket connections running at the same time.

syntax for otp_d:
    otp_d listening_port


## otp.c

This program connects to otp_d and asks it to store or retrieve messags for a given user. It has two modes: ost and get.

In post mode, otp will encrypt plaintext using key then send user and the encrypted message to otp_d
syntax for otp:
    otp post user plaintext key port

user: the name you want otp_d to assocate with the encrypted message.
plaintext: the name of the file that contain the plaintext you wish to encrypt
key: contains the encryption key to use to encrypt the text
port: the port that otp shuould attempt to connect to otp_d on.

In get mode, otp will send a request for a message for user. otp will use key to decrypt the message and print the decrypted message to stdout.
If the user does not have any message, otp should report an error stderr

snyntax for otp
    otp get user key port
    otp get user key port > myciphertext
    otp get user key port > myciphertext &

user: the name you want otp_d to retrteve an encrypted message for.
key: contains the decryption key to use to decrypt the retrieved ciphertext.
port: the port that otp shuould attempt to connect to otp_d on.

## To compile the files:

    gcc -o keygen keygen.c
    gcc -o otp otp.c
    gcc -o otp_d otp_d.c
