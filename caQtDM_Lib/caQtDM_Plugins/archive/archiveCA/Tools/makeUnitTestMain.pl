use English;
use File::Basename;

my ($opt_v) = 0;
my (%test_units);

sub parse($)
{
    my ($filename) = @ARG;
    my ($test_unit, $test_case);

    return if $filename eq "UnitTest.cpp";

    open(F,$filename) or die "Cannot read '$filename'\n";
    $test_unit = basename($filename, ".cpp");
    print "Parsing '$filename', unit '$test_unit'\n" if ($opt_v);
    while (<F>)
    {
        if ($ARG =~ '\s*TEST_CASE (\S+)\(')
        {
            $test_case = $1;
            print "+ Test Case '$test_case'\n" if ($opt_v);
            push @{ $test_units{$test_unit} }, $test_case;
        }
    }
    close(F);
}

foreach (<*.cpp>)
{
    parse($ARG);
}

######################################
printf("Creating UnitTest.mk\n");
######################################
open(F, ">UnitTest.mk") or die "Cannot create UnitTest.mk\n";
select(F);

printf("# Created by makeUnitTestMain.pl, do NOT modify!\n");
printf("\n");
printf("TESTPROD_HOST += UnitTest\n");
printf("\n");
foreach $test_unit ( sort keys %test_units )
{
    printf("UnitTest_SRCS += $test_unit.cpp\n");
}
printf("UnitTest_SRCS += UnitTest.cpp\n");

close(F);

######################################
printf("Creating UnitTest.cpp\n");
######################################
open(F, ">UnitTest.cpp") or die "Cannot create UnitTest.cpp\n";
select(F);

printf("// UnitTest Suite,\n");
printf("// created by makeUnitTestMain.pl.\n");
printf("// Do NOT modify!\n");
printf("\n");
printf("// System\n");
printf("#include <stdio.h>\n");
printf("#include <string.h>\n");
printf("// Tools\n");
printf("#include <UnitTest.h>\n");
printf("\n");
foreach $test_unit ( sort keys %test_units )
{
    printf("// Unit $test_unit:\n");
    foreach $test_case ( @{ $test_units{$test_unit}} )
    {
        print "extern TEST_CASE $test_case();\n";
    }
}
printf("\n");
printf("int main(int argc, const char *argv[])\n");
printf("{\n");
printf("    size_t units = 0, run = 0, passed = 0;\n");
printf("    const char *single_unit = 0;\n");
printf("    const char *single_case = 0;\n");
printf("\n");
printf("    if (argc > 3  || (argc > 1 && argv[1][0]=='-'))\n");
printf("    {\n");
printf("        printf(\"USAGE: UnitTest { Unit { case } }\\n\");\n");
printf("        printf(\"\\n\");\n");
printf("        printf(\"Per default, all test cases in all units are executed.\\n\");\n");
printf("        printf(\"\\n\");\n");
printf("        return -1;\n");
printf("    }\n");
printf("    if (argc >= 2)\n");
printf("        single_unit = argv[1];\n");
printf("    if (argc == 3)\n");
printf("        single_case = argv[2];\n");
printf("\n");
foreach $test_unit ( sort keys %test_units )
{
    printf("    if (single_unit==0  ||  strcmp(single_unit, \"$test_unit\")==0)\n");
    printf("    {\n");

    printf("        printf(\"======================================================================\\n\");\n");
    printf("        printf(\"Unit $test_unit:\\n\");\n");
    printf("        printf(\"----------------------------------------------------------------------\\n\");\n");
    printf("        ++units;\n");
    foreach $test_case ( @{ $test_units{$test_unit}} )
    {

        printf("       if (single_case==0  ||  strcmp(single_case, \"$test_case\")==0)\n");
        printf("       {\n");


        print("            ++run;\n");
        print("            printf(\"\\n$test_case:\\n\");\n");
        print("            if ($test_case())\n");
        print("                ++passed;\n");
        print("            else\n");
        print("                printf(\"THERE WERE ERRORS!\\n\");\n");
        printf("       }\n");



    }
    printf("    }\n");
}
printf("\n");
printf("    printf(\"======================================================================\\n\");\n");
printf("    size_t failed = run - passed;\n");
printf("    printf(\"Tested %zu unit%%s, ran %%zu test%%s, %%zu passed, %%zu failed.\\n\",\n");
printf("           units,\n");
printf("           (units > 1 ? \"s\" : \"\"),\n");
printf("           run,\n");
printf("           (run > 1 ? \"s\" : \"\"),\n");
printf("           passed, failed);\n");
printf("    printf(\"Success rate: %%.1f%%%%\\n\", 100.0*passed/run);\n");
printf("\n");
printf("    printf(\"==================================================\\n\");\n");
printf("    printf(\"--------------------------------------------------\\n\");\n");
printf("    if (failed != 0)\n");
printf("    {\n");
printf("        printf(\"THERE WERE ERRORS!\\n\");\n");
printf("        return -1;\n");
printf("    }\n");
printf("    printf(\"All is OK\\n\");\n");
printf("    printf(\"--------------------------------------------------\\n\");\n");
printf("    printf(\"==================================================\\n\");\n");
printf("\n");
printf("    return 0;\n");
printf("}\n");
printf("\n");

