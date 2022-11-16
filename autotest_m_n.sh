#!/usr/bin/bash


rm output_autotest.txt;
for ((i = 2; i <= $1; i++))   #M
    do
        for ((j = 2; j <= $2; j++))   #N
            do  
                for ((k = 4; k <= 32; k++))
                    do
                        for ((l = 0; l <= 1; l++))
                            do
                                echo "TestStart - $i X $j _$k _$l";
                                echo "TestStart -  $i X $j _$k _$l" >> output_autotest.txt;
                                ./testmode1 $i $j $k $l >> output_autotest.txt;
                                echo "TestEnd - $i X $j _$k _$l";
                                echo "TestEnd -  $i X $j _$k _$l" >> output_autotest.txt;
                            done
                    done
            done
    done