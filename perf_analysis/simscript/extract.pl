use strict;
use warnings;

my $count = 0;
my $countinternal = 0;
my @words;
open (my $inFile, '<', 'val.txt') or die $!;

while (<$inFile>) {
  chomp;
  @words = split(' ');
  if($count == 0){
    printf("Benchmark,RRS,Scale-SRS");
  }
  $countinternal = 0;
  foreach my $word (@words) {
    if(($count > 1) && ($count < 32)){
        if($countinternal != 1){
            printf("$word");
        }
        if(($countinternal == 0) || ($countinternal == 2)){
            printf(",");
        }
    }
    if($count == 65){
        if($countinternal == 0){
            printf("SPEC2006");
        }
        if($countinternal > 1){
            printf("$word");
        }
        if(($countinternal == 0) || ($countinternal == 2)){
            printf(",");
        }
    }
    if($count == 92){
        if($countinternal == 0){
            printf("SPEC2017");
        }
        if($countinternal > 1){
            printf("$word");
        }
        if(($countinternal == 0) || ($countinternal == 2)){
            printf(",");
        }
    }
    if($count == 103){
        if($countinternal == 0){
            printf("GAP");
        }
        if($countinternal > 1){
            printf("$word");
        }
        if(($countinternal == 0) || ($countinternal == 2)){
            printf(",");
        }
    }
    if($count == 113){
        if($countinternal == 0){
            printf("COMM");
        }
        if($countinternal > 1){
            printf("$word");
        }
        if(($countinternal == 0) || ($countinternal == 2)){
            printf(",");
        }
    }
    if($count == 125){
        if($countinternal == 0){
            printf("PARSEC");
        }
        if($countinternal > 1){
            printf("$word");
        }
        if(($countinternal == 0) || ($countinternal == 2)){
            printf(",");
        }
    }
    if($count == 132){
        if($countinternal == 0){
            printf("BIOBENCH");
        }
        if($countinternal > 1){
            printf("$word");
        }
        if(($countinternal == 0) || ($countinternal == 2)){
            printf(",");
        }
    }
    if($count == 143){
        if($countinternal == 0){
            printf("MIX");
        }
        if($countinternal > 1){
            printf("$word");
        }
        if(($countinternal == 0) || ($countinternal == 2)){
            printf(",");
        }
    }
    if($count == 226){
        if($countinternal == 0){
            printf("GMEAN");
        }
        if($countinternal > 1){
            printf("$word");
        }
        if(($countinternal == 0) || ($countinternal == 2)){
            printf(",");
        }
    }
    $countinternal++; 
  }
  if(($count > 1) && ($count < 32)){
    printf("\n");
  }
  if($count == 64){
    printf(" ,,\n");
  }
  if($count == 65){
    printf("\n");
  }
  if($count == 92){
    printf("\n");
  }
  if($count == 103){
    printf("\n");
  }
  if($count == 113){
    printf("\n");
  }
  if($count == 125){
    printf("\n");
  }
  if($count == 132){
    printf("\n");
  }
  if($count == 143){
    printf("\n");
  }
  if($count == 226){
    printf("\n");
  }
  $count++;
}

close ($inFile);
