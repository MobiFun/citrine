#!/usr/bin/perl -w
#
# mktarget.pl by Mads Meisner-Jensen (mmj@ti.com)
#
# Auto-generate target code from ffs.c source file.
#

$FFSPATH = shift;
if (!defined($FFSPATH) || $FFSPATH eq "") {
    $FFSPATH = ".";
}

$OUTPATH = shift;
if (!defined($OUTPATH) || $OUTPATH eq "") {
  $OUTPATH = $FFSPATH;
}

$verbose = 1;

$STATE_OUTSIDE = 0;
$STATE_IN_FUNCTION = 1;
$STATE_IN_TASK = 2;
$state = $STATE_OUTSIDE;


############################################################################
# Main
############################################################################

$verbose > 0 && print "mktarget.pl operating in directory '$FFSPATH'\n";

open (FFS, "<$FFSPATH/ffs.c") || die "Failed to open 'ffs.c'";
print $OUTPATH;
open (TARGETC, ">$OUTPATH/ffs_target.c") || die "Failed to open 'ffs_target.c'";

target_put("/**********************************************************\n");
target_put(" *\n");
target_put(" * This is an auto-generated file. Do NOT edit!\n");
target_put(" *\n");
target_put(" **********************************************************/\n\n\n");

while (<FFS>)
{
    if ($state == $STATE_OUTSIDE)
    {
        target_put($_);
        if (/^{\s*$/) {
            $state = $STATE_IN_FUNCTION;
        }
    }
    elsif ($state == $STATE_IN_FUNCTION)
    {
        if (/^\s*\/\/\s*TASKBEGIN\s*(\w+)\s*(\w+)\s*\((.*)\)(.*)$/m)
        {
            $state = $STATE_IN_TASK;

            $rettype = $1;
            $func = $2;
            $funclc = lc($func);
            
            @args = split(',', $3);
            @values = ();
            @reqmembers = ();
            foreach (@args) {
                ($left, $right) = $_ =~ /(\w+)=(.+)/m;
                push (@reqmembers, $left);
                push (@values, $right);
            }
            
            @vars = split(';', $4);
            
            task_begin();
        }
        else
        {
            target_put($_);
            if (/^}\s*$/m) {
                $state = $STATE_OUTSIDE;

                # Write the task_* code after the function
                target_put("\n");
                target_put(@taskcode);
                @taskcode = ();
            }
        }
    }
    elsif ($state == $STATE_IN_TASK)
    {
        if (/^.*TASKEND.*$/m)
        {
            $state = $STATE_IN_FUNCTION;

            foreach (@values) {
                if (/^[a-zA-Z]+/m) {
                    task_put("#undef $_\n");
                }
            }
            task_put("\n");
#            task_put("    return EFFS_OK;\n");
            task_put("}\n\n");
        }
        else {
            task_put($_);
        }
    }
    else
    {
        die "Bad state!";
    }
}

close (FFS) || die;
close (TARGETC) || die;

sub task_put
{
    push (@taskcode, @_);
}

sub target_put
{
    print TARGETC @_;
}

sub task_begin
{
    $verbose > 0 && print "ffs_$funclc ($func)\n";
    $verbose > 1 && print "  args = @args\n";
    $verbose > 1 && print "    left = @values, right = @reqmembers\n";
    $verbose > 1 && print "  vars = @vars\n";
    $verbose > 1 && print "\n";

    task_put("$rettype task_$funclc(struct ffs_req_s *p)\n");
    task_put("{\n");
    foreach (@vars) {
        task_put("   $_;\n");
    }
    task_put("\n");
    for $i (0..$#args) {
        # If value starts with alpha char, e.g. a variable name
        if ($values[$i] =~ /^[a-zA-Z]+/m) {
            task_put("#define $values[$i] p->$reqmembers[$i]\n");
        }
    }

    target_put("    {\n");
    target_put("        struct ffs_req_s *req;\n");
    target_put("        MSG_ALLOC(req);\n");

    for $i (0..$#args) {
        # Typecasting is necessary for the sake of the tms470 compiler
        if ($reqmembers[$i] eq "src") {
            target_put("        req->$reqmembers[$i] = (char *) $values[$i];\n");
        }
        else {
            target_put("        req->$reqmembers[$i] = $values[$i];\n");
        }
    }
    target_put("        req->request_id = request_id_get();\n");     
    target_put("        req->fb = fb;\n");     
    target_put("        req->cp = cp;\n");
    target_put("        req->cmd = $func;\n");
    target_put("        MSG_SEND(req);\n");
    target_put("        return req->request_id;\n");
    target_put("    }\n");

}
