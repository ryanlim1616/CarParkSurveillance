cls

$gt = Import-Csv C:\powershell\gt.csv
$db = Import-Csv C:\powershell\tracks_db.csv
$time_window = [timespan]('00:00:05') 
$count = 0
$duplicate_check = $FALSE
$FirstRun = $TRUE

$true_positive = 0
$true_negative = 0
$false_positive = 0
$false_negative = 0

$precision = 0
$recall = 0
$accuracy = 0

#Calculate F score, beta value set to 1
$f_score = 0
$f_beta = 1;

##########################################################
### TEMPORARY

$output = $gt[0].t_time + ","+ $gt[0].o_state


##########################################################

write-host "Processing records:"


"gt_time" +"," +"gt_state"+ ","+ "db_time"+ "," +"db_state" | out-file C:\powershell\output.txt



for($i = 0; $i -lt $gt.length; $i++){

    for($j = 0; $j -lt $db.Length; $j++)
    {

        if($gt[$i].o_state  -eq $db[$j].obj_state)
        {
            
            if([datetime]$gt[$i].t_time + $time_window -ge [datetime]$db[$j].track_time -and    
                [datetime]$gt[$i].t_time - $time_window -le [datetime]$db[$j].track_time)
                {

                #write-host $($gt[$i].t_time)"," $($gt[$i].o_state)"," $($db[$j].track_time)"," $($db[$j].obj_state) | out-file output.txt
                #$($gt[$i].t_time)+ ","+ $($gt[$i].o_state) | out-file C:\powershell\output.txt -Append
                #$output += (($($gt[$i].t_time)+ ","+ $($gt[$i].o_state))+","+( $($db[$j].track_time)+","+ $($db[$j].obj_state)))

                    $temp1 = $($gt[$i].t_time)+ ","+ $($gt[$i].o_state)
                    if($output -ne $temp1 -or $FirstRun)
                    {
                        $($gt[$i].t_time)+ ","+ $($gt[$i].o_state)+"," +$($db[$j].track_time)+","+ $($db[$j].obj_state) | out-file C:\powershell\output.txt -Append
                        $output = ($($gt[$i].t_time)+ ","+ $($gt[$i].o_state))
                        $true_positive++
                        $FirstRun = $false;

                    }

                        if($gt[$i].o_state -eq $gt[$i+1].o_state -or `
                            $db[$j].obj_state -eq $db[$j+1].obj_state `
                            -and [datetime]$db[$j].track_time + $time_window -ge [datetime]$db[$j+1].track_time `
                            -and [datetime]$db[$j].track_time - $time_window -le [datetime]$db[$j+1].track_time)
                        {
                            $duplicate_check = $TRUE;
                            $j++
                        }
      
                } 
             else
                {
                    $duplicate_check = $FALSE;
                }      
        }
        else
        {
            $duplicate_check = $FALSE;
        }
          
        if($duplicate_check)
        {
            $j++
        }

    }


} 


$false_positive = $db.Length - $true_positive
$false_negative = $gt.Length - $true_positive


write-host "Total number of matched: "$($true_positive) 


"" | out-file C:\powershell\output.txt -Append
"Summary: " | out-file C:\powershell\output.txt -Append

"---------------------------------------------" | out-file C:\powershell\output.txt -Append

"True Positive: "  + $($true_positive)  | out-file C:\powershell\output.txt -Append
"False Positive: " + $($false_positive) | out-file C:\powershell\output.txt -Append
"False Negative: " + $($false_negative) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append

$precision = $true_positive / ($true_positive + $false_positive) 
$recall = $true_positive / ($true_positive + $false_negative) 
$accuracy = ($true_positive)/($true_positive + $false_positive + $false_negative) 

$f_score = (1+$f_beta) * (($precision * $recall) / ($precision + $recall))

"Precision: " + $($precision* 100) | out-file C:\powershell\output.txt -Append
"Recall : " + $($recall* 100) | out-file C:\powershell\output.txt -Append
"Accuracy :" + $($accuracy* 100) | out-file C:\powershell\output.txt -Append
"F score (beta=" + $($f_beta) + "): " + $($f_score) | out-file C:\powershell\output.txt -Append


#############################################################


