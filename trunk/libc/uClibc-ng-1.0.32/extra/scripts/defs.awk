/^[ 	]*\.endp/	 { need_endp = 1 }
/^[ 	]*\.end/	 { need_end = 1 }
/^[ 	]*\.align/ { if($2 > max) max = $2; }

END {
    if(need_endp)
    {
	print "#define END_INIT .endp _init";
	print "#define END_FINI .endp _fini";
    } else if(need_end)
    {
	print "#define END_INIT .end _init";
	print "#define END_FINI .end _fini";
    }
    else
    {
	print "#define END_INIT";
	print "#define END_FINI";
    }
    if(max)
	print "#define ALIGN .align", max;
    else
	print "#define ALIGN";

    print "#include <libc-symbols.h>";
    #print "weak_extern (__gmon_start__)";
}
