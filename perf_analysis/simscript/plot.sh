if [ -f val.txt ]; then
   rm val.txt
fi
if [ -f data.csv ]; then
   rm data.csv
fi
touch val.txt
touch data.csv
./getdata.pl -s ADDED_IPC -w interest_name -n 0 -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ >> val.txt
./getdata.pl -s ADDED_IPC -w spec2006_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ >> val.txt
./getdata.pl -s ADDED_IPC -w spec2017_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ >> val.txt
./getdata.pl -s ADDED_IPC -w gap_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ >> val.txt
./getdata.pl -s ADDED_IPC -w comm_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ >> val.txt
./getdata.pl -s ADDED_IPC -w parsec_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ >> val.txt
./getdata.pl -s ADDED_IPC -w biobench_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ >> val.txt
./getdata.pl -s ADDED_IPC -w mix_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ >> val.txt
./getdata.pl -s ADDED_IPC -w all78 -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ >> val.txt
perl extract.pl >> data.csv
rm val.txt
