#!/usr/bin/perl -w

require ( "./bench_common_srs1K.pl");

$trace_dir = "../";


#####################################
######### USAGE OPTIONS      ########
#####################################

$wsuite   = "testA";
$isuite   = "testA_name";
$sim_exe  = "../bin/usimm";
$dest_dir  = ".";
$sim_opt   = "  ";
$debug     = 0;
$firewidth = 2; # num parallel jobs at a time
$sleep     = 0;

#####################################
######### USAGE OPTIONS      ########
#####################################

sub usage(){

$USAGE = "Usage:  '$0 <-option> '";
 
print(STDERR "$USAGE\n");
print(STDERR "\t--h                    : help -- print this menu. \n");
print(STDERR "\t--r                    : rerun -- re-execute killed sims. \n");
print(STDERR "\t--d <dest_dir>         : name of the destination dir. \n");
print(STDERR "\t--w <workload/suite>   : workload suite from bench_common \n");
print(STDERR "\t--i <input/suite>      : input suite from bench_common_srs1K \n");
print(STDERR "\t--s <sim_exe>          : simulator executable \n");
print(STDERR "\t--o <sim_options>      : simulator options \n");
print(STDERR "\t--dbg                  : debug \n");
print(STDERR "\t--f <val>              : firewidth, num of parallel jobs \n");
print(STDERR "\t--z <val>              : sleep for z seconds \n");
print(STDERR "\n");

exit(1);
}

######################################
########## PARSE COMMAND LINE ########
######################################
 
while (@ARGV) {
    $option = shift;
 
    if ($option eq "--dbg") {
        $debug = 1;
    }elsif ($option eq "--w") {
        $wsuite = shift;
    }elsif ($option eq "--i") {
        $isuite = shift;
    }elsif ($option eq "--s") {
        $sim_exe = shift;
    }elsif ($option eq "--o") {
        $sim_opt = shift;
    }elsif ($option eq "--d") {
        $dest_dir = shift;
    }elsif ($option eq "--f") {
        $firewidth = shift;
    }elsif ($option eq "--z") {
        $sleep = shift;
    }elsif ($option eq "--e") {
        $sim_exe = shift;
    }else{
	usage();
        die "Incorrect option ... Quitting\n";
    }
}


##########################################################             
# create full path to mysim and dest dir

$mysim = "$dest_dir"."/"."sim_srs1K.bin";
#$myopt = "$dest_dir"."/"."sim_srs1K.opt";

#print "$mysim\n";

##########################################################
# get the suite names, num_w, and num_p from bench_common

die "No benchmark set '$wsuite' defined in bench_common_srs1K.pl\n"
    unless $SUITES{$wsuite};
@w = split(/\n/, $SUITES{$wsuite});
$num_w = scalar @w;


die "No benchmark set '$isuite' defined in bench_common_srs1K.pl\n"
    unless $SUITES{$isuite};
@i = split(/\n/, $SUITES{$isuite});

##########################################################

if($debug==0){
    mkdir "$dest_dir";
    #system ("ls $dest_dir");
    system ("cp -f $sim_exe $mysim");
    system ("chmod +x $mysim");
    #system ("echo $sim_opt > $myopt");
}

##########################################################

for($ii=0; $ii< $num_w; $ii++){
    $iname = $i[$ii];
    $wname = $w[$ii];

    $infiles = " ";

    @bmks = split(/ /,$wname);
    
    foreach (@bmks) {
	$bname   = $_;
	$infile  = $trace_dir.$bname;
	$infiles = $infiles." ".$infile;
    }
    $outfile = " > $dest_dir/".$iname;
   
    $exe = "$mysim $sim_opt $infiles $outfile";
    #$exe = "$mysim $sim_opt $infiles ";
    #echo $exe

    #background all jobs except the last one (acts as barrier)
    $exe .= " &" unless($ii%$firewidth==($firewidth-1));
    print "$exe\n";
    system("$exe") unless($debug);
}
