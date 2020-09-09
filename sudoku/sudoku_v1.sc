# install scopes (http://scopes.rocks)
# compile with: scopes sudoku_v1.sc && gcc sudoku_v1.sc.o sudoku_v1.sc.c.o -o sudoku_v1.sc.exe
# run with: ./sudoku_v1.sc.exe < sudoku.txt

let libC = 
    include 
        options "-c" "sudoku_v1.sc.c.o"
        """"#include <stdio.h>
            #include <string.h>
            typeof(stdin) get_stdin() { return stdin; }

let libC =
    do
        using libC.extern
        using libC.typedef
        locals;

global R : (array (array u16 9) 324)
global C : (array (array u16 4) 729)

fn genmat ()
    local r = 0
    for i in (range 9)
        for j in (range 9)
            for k in (range 9)
                let cr = (C @ r)
                cr @ 0 = (9 * i + j) as u16
                cr @ 1 = ((i // 3 * 3 + j // 3) * 9 + k + 81) as u16
                cr @ 2 = (9 * i + k + 162) as u16
                cr @ 3 = (9 * j + k + 243) as u16
                r = r + 1
    
    local nr : (array i8 324)
    for c in (range 324)
        nr @ c = 0

    for r in (range 729)
        for c2 in (range 4)
            let k = (C @ r @ c2)
            (R @ k) @ (nr @ k) = r as u16
            nr @ k = nr @ k + 1:u16

fn update (sr sc r v)
    let v7 = (v << 7)
    for c2 in (range 4)
        let x = (sc @ (C @ r @ c2))
        x = (x + v7) as u8
    local min = 10
    local min_c = 0
    for c2 in (range 4)
        let c = (C @ r @ c2)
        if (v > 0)
            for r2 in (range 9)
                let rr = (R @ c @ r2)
                local srrr = (sr @ rr)
                sr @ rr = srrr + 1
                if (srrr != 0)
                    continue;
                for cc2 in (range 4)
                    let cc = (C @ rr @ cc2)
                    sc @ cc = sc @ cc - 1
                    if (sc @ cc < min)
                        min = sc @ cc
                        min_c = cc
        else
            for r2 in (range 9)
                let rr = (R @ c @ r2)
                sr @ rr = sr @ rr - 1
                if (sr @ rr != 0)
                    continue;
                let p = (C @ rr)
                sc @ (p @ 0) = sc @ (p @ 0) + 1
                sc @ (p @ 1) = sc @ (p @ 1) + 1
                sc @ (p @ 2) = sc @ (p @ 2) + 1
                sc @ (p @ 3) = sc @ (p @ 3) + 1
    min << 16 | min_c
    
fn solve (_s)
    local sr : (array i8 729)
    for r in (range 729)
        sr @ r = 0
    
    local sc : (array u8 324)
    for c in (range 324)
        sc @ c = 0 << 7 | 9
    
    let nine = 57:i8
    let one = 49:i8
    local hints = 0
    local cr : (array i8 81)
    local cc : (array i16 81)
    local out : (array i8 82)
    for i in (range 81)
        let c = (_s @ i)
        let a = 
            if (c >= one and c <= nine)
                c - one
            else
                -1:i8
        if (a >= 0)
            update sr sc (i * 9 + a) 1
            hints = hints + 1
        cr @ i = -1
        cc @ i = -1
        out @ i = c
    out @ 81 = 0

    local n = 0
    local min : i32
    local i = 0
    local dir = 1
    local cand = (10 << 16 | 0)
    loop ()
        while (i >= 0 and i < 81 - hints)
            if (dir == 1)
                min = (cand >> 16) as i32
                cc @ i = (cand & 0xFFFF) as i16
                if (min > 1)
                    for c in (range 324)
                        if (sc @ c < min)
                            min = sc @ c
                            cc @ i = c as i16
                            if (min <= 1)
                                break;
                if (min == 0 or min == 10)
                    cr @ i = -1
                    dir = -1
                    i = i - 1
            let c = (cc @ i)
            if (dir == -1 and cr @ i >= 0)
                update sr sc (R @ c @ (cr @ i)) -1
            let r2 = 
                loop (r2 = (cr @ i + 1))
                    if (r2 >= 9)
                        break r2
                    if (sr @ (R @ c @ r2) == 0)
                        break r2
                    repeat (r2 + 1)
            if (r2 < 9)
                cand = (update sr sc (R @ c @ r2) 1)
                cr @ i = r2
                i = i + 1
                dir = 1
            else
                cr @ i = -1
                i = i - 1
                dir = -1
        if (i < 0)
            break;
        for j in (range i)
            let r = (R @ (cc @ j) @ (cr @ j))
            out @ (r // 9) = (r % 9 + one) as i8
        libC.puts out
        n = n + 1
        i = i - 1
        dir = -1
        repeat;
    n

fn sudoku_solver ()
    genmat;
    let stdin = (libC.get_stdin)
    local buf : (array i8 1024)
    libC.fgets buf 1024 stdin
    let newline = 10:i8
    while ((libC.fgets buf 1024 stdin) != null)
        if ((libC.strlen buf) < 81)
            continue;
        solve buf
        libC.putchar newline

fn main (argc argv)
    sudoku_solver;
    0

#sudoku_solver;

compile-object
    default-target-triple
    compiler-file-kind-object
    module-dir .. "/sudoku_v1.sc.o"
    do
        let main =
            static-typify main i32 (pointer rawstring)
        locals;
    'O3
