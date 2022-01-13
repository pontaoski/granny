This program writes "World" to a file called "Hello".

---

|1 &main Jump ( entrypoint )

~"files.orc" ( files namespace )

:main ( the main routine )

    &data:Filename #1 !file:Open

    &data:Content !file:WriteString

    Halt

:data
    @Filename "Hello"
    @Content "World"

---