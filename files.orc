File IO functions.

---

:file ( file functions )

    @Open ( mode string -> handle )
        #0 #1 IOAct Ret

    @WriteByte ( byte handle -> )
        #3 #1 IOAct Ret

    @WriteString ( stringPointer handle -> )

    @loop ( stringPointer handle -> )
        Duplicate LoadFrom ( Get the char... )
        #0 Equals ( Does it equal NULL? )

        &file:exitloop Swap CondJump ( If it does, jump to the end, since we've written all the bytes )

        ( stringPointer handle )

        Over ( handle stringPointer handle )
        Over ( stringPointer handle stringPointer handle )

        ( Get the char again )
        LoadFrom

        ( char handle stringPointer handle )
        !file:WriteByte
        ( stringPointer handle )
        Increment

        &file:loop Jump

    @exitloop
        Ret

---
