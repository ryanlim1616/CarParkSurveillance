cls

$gt = Import-Csv C:\powershell\gt.csv

#db250317
#db260317_skip
#tracks_db
#db_260317_rerun

#290317db.csv

$db = Import-Csv C:\powershell\withYOLO.csv
$time_window = [timespan]('00:00:05') 
$time_dec = [timespan]('00:00:05')
$write = $false
$temp_J = 0

##########################################################
### TEMPORARY

"-- START OF JOB --" | out-file C:\powershell\output.txt 

##########################################################

write-host "Processing records:"

###### START #####
###### COMMENT THE FOLLOWING LINES IF NOT USING Multiple Time Window #######
while($time_window -ge $time_dec) 
{


$count = 0;
$duplicate_check = $FALSE;
$FirstRun = $TRUE;

$true_positive = 0;
$true_negative = 0;
$false_positive = 0;
$false_negative = 0;

$precision = 0;
$recall = 0;
$accuracy = 0;

#Calculate F score, beta value set to 1
$f_score = 0;
$f_beta = 1;

###########################
# for individual states
###########################


$totaldb_enter = 0;
$totalgt_enter = 0;
$true_positive_enter = 0;
$true_negative_enter = 0;
$false_positive_enter = 0;
$false_negative_enter = 0;

$precision_enter = 0;
$recall_enter = 0;
$accuracy_enter = 0;
$f_score_enter = 0;

####
$totaldb_exit = 0;
$totalgt_exit = 0;
$true_positive_exit = 0;
$true_negative_exit = 0;
$false_positive_exit = 0;
$false_negative_exit = 0;

$precision_exit = 0;
$recall_exit = 0;
$accuracy_exit = 0;
$f_score_exit = 0;

####
$totaldb_enterNTZ = 0;
$totalgt_enterNTZ = 0;
$true_positive_enterNTZ = 0;
$true_negative_enterNTZ = 0;
$false_positive_enterNTZ = 0;
$false_negative_enterNTZ = 0;

$precision_enterNTZ = 0;
$recall_enterNTZ = 0;
$accuracy_enterNTZ = 0;
$f_score_enterNTZ = 0;

####
$totaldb_exitNTZ = 0;
$totalgt_exitNTZ = 0;
$true_positive_exitNTZ = 0;
$true_negative_exitNTZ = 0;
$false_positive_exitNTZ = 0;
$false_negative_exitNTZ = 0;

$precision_exitNTZ = 0;
$recall_exitNTZ = 0;
$accuracy_exitNTZ = 0;
$f_score_exitNTZ = 0;

####
$totaldb_park = 0;
$totalgt_park = 0;
$true_positive_park = 0;
$true_negative_park = 0;
$false_positive_park = 0;
$false_negative_park = 0;

$precision_park = 0;
$recall_park = 0;
$accuracy_park = 0;
$f_score_park = 0;

####
$totaldb_leave = 0;
$totalgt_leave = 0;
$true_positive_leave = 0;
$true_negative_leave = 0;
$false_positive_leave = 0;
$false_negative_leave = 0;

$precision_leave = 0;
$recall_leave = 0;
$accuracy_leave = 0;
$f_score_leave = 0;

###########################


#initialize to first ground truth data
$output = $gt[0].t_time + ","+ $gt[0].o_state


"##########################################################" | out-file C:\powershell\output.txt -Append
"Time Window: " + $($time_window) | out-file C:\powershell\output.txt -Append

"gt_time" +"," +"gt_state"+ ","+ "db_time"+ "," +"db_state" | out-file C:\powershell\output.txt -Append


for($k = 0; $k -lt $db.Length; $k++){

            
            	if($db[$k].obj_state -like "*Enter:*")
                {
                    $totaldb_enter++;
                }                 
            
                if($db[$k].obj_state -like "*Park : Lot *")
                {
                    $totaldb_park++;
                }          
            
                if($db[$k].obj_state -like "*Park : NTZ*")
                {
                    $totaldb_enterNTZ++;
                }
            
                if($db[$k].obj_state -like "*Leave: NTZ*")
                {
                    $totaldb_exitNTZ++;
                }
                elseif($db[$k].obj_state -like "*Leave: Lot*")
                {
                    $totaldb_leave++;
                }   
                elseif($db[$k].obj_state -like "*Leave: *")
                {
                    $totaldb_exit++;
                }        
            

}



for($i = 0; $i -lt $gt.length; $i++){

    
    if($gt[$i].o_state -like "*Enter:*")
    {
        $totalgt_enter++;
    }                 
    
    if($gt[$i].o_state -like "*Park : Lot *")
    {
        $totalgt_park++;
    }          
    
    if($gt[$i].o_state -like "*Park : NTZ*")
    {
        $totalgt_enterNTZ++;
    }
    
    if($gt[$i].o_state -like "*Leave: NTZ*")
    {
        $totalgt_exitNTZ++;
    }
    elseif($gt[$i].o_state -like "*Leave: Lot*")
    {
        $totalgt_leave++;
    }   
    elseif($gt[$i].o_state -like "*Leave: *")
    {
        $totalgt_exit++;
    }    


    $written_once = $FALSE;



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
                        $FirstRun = $false
                        $write = $FALSE
                        $written_once = $TRUE;
                        $temp_J = $j

                        ## for individual states

                        if($gt[$i].o_state -like "*Enter:*")
                        {
                            $true_positive_enter++;
                        }                 

                        if($gt[$i].o_state -like "*Park : Lot *")
                        {
                            $true_positive_park++;
                        }          

                        if($gt[$i].o_state -like "*Park : NTZ*")
                        {
                            $true_positive_enterNTZ++;
                        }

                        if($gt[$i].o_state -like "*Leave: NTZ*")
                        {
                            $true_positive_exitNTZ++;
                        }
                        elseif($gt[$i].o_state -like "*Leave: Lot*")
                        {
                            $true_positive_leave++;
                        }   
                        elseif($gt[$i].o_state -like "*Leave: *")
                        {
                            $true_positive_exit++;
                        }    
                                      


                        ##

                    }

                        if($gt[$i].o_state -eq $gt[$i+1].o_state -or `
                            $db[$j].obj_state -eq $db[$j+1].obj_state `
                            -and [datetime]$db[$j].track_time + $time_window -ge [datetime]$db[$j+1].track_time `
                            -and [datetime]$db[$j].track_time - $time_window -le [datetime]$db[$j+1].track_time)
                        {
                            $duplicate_check = $TRUE
                            $j++
                        }
      
                } 
             else
                {
                    $duplicate_check = $FALSE                   
                }      
        }
        else
        {
            $duplicate_check = $FALSE;
        }
        

		if($written_once -eq $FALSE)
		{

                
            if($j -gt $temp_J -and ([datetime]$gt[$i].t_time -gt [datetime]$db[$j].track_time))
            {
                $temp_J = $j

				","+ "," +$($db[$j].track_time)+","+ $($db[$j].obj_state) | out-file C:\powershell\output.txt -Append
                $written_once = $TRUE;
			}
        }
		
        if($duplicate_check)
        {
            $j++
        }

    }

      if($write -eq $TRUE)
      {
          $($gt[$i].t_time)+ ","+ $($gt[$i].o_state)+"," +"," | out-file C:\powershell\output.txt -Append
          #","+ "," +$($db[$j].track_time)+","+ $($db[$j].obj_state) | out-file C:\powershell\output.txt -Append
          $write = $FALSE;
      }

    $write = $TRUE


} 


$false_positive = $db.Length - $true_positive
$false_negative = $gt.Length - $true_positive


$false_positive_enter = $totaldb_enter - $true_positive_enter
$false_positive_exit = $totaldb_exit - $true_positive_exit
$false_positive_enterNTZ = $totaldb_enterNTZ - $true_positive_enterNTZ
$false_positive_exitNTZ = $totaldb_exitNTZ - $true_positive_exitNTZ
$false_positive_park = $totaldb_park - $true_positive_park
$false_positive_leave = $totaldb_leave - $true_positive_leave


$false_negative_enter = $totalgt_enter - $true_positive_enter
$false_negative_exit = $totalgt_exit - $true_positive_exit
$false_negative_enterNTZ = $totalgt_enterNTZ - $true_positive_enterNTZ
$false_negative_exitNTZ = $totalgt_exitNTZ - $true_positive_exitNTZ
$false_negative_park = $totalgt_park - $true_positive_park
$false_negative_leave = $totalgt_leave - $true_positive_leave












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

####

$precision_enter = $true_positive_enter / ($true_positive_enter + $false_positive_enter) 
$recall_enter = $true_positive_enter / ($true_positive_enter + $false_negative_enter) 
$accuracy_enter = ($true_positive_enter)/($true_positive_enter + $false_positive_enter + $false_negative_enter) 
$f_score_enter = (1+$f_beta) * (($precision_enter * $recall_enter) / ($precision_enter + $recall_enter))


$precision_exit = $true_positive_exit / ($true_positive_exit + $false_positive_exit) 
$recall_exit = $true_positive_exit / ($true_positive_exit + $false_negative_exit) 
$accuracy_exit = ($true_positive_exit)/($true_positive_exit + $false_positive_exit + $false_negative_exit) 
$f_score_exit = (1+$f_beta) * (($precision_exit * $recall_exit) / ($precision_exit + $recall_exit))


$precision_enterNTZ = $true_positive_enterNTZ / ($true_positive_enterNTZ + $false_positive_enterNTZ) 
$recall_enterNTZ = $true_positive_enterNTZ / ($true_positive_enterNTZ + $false_negative_enterNTZ) 
$accuracy_enterNTZ = ($true_positive_enterNTZ)/($true_positive_enterNTZ + $false_positive_enterNTZ + $false_negative_enterNTZ) 
$f_score_enterNTZ = (1+$f_beta) * (($precision_enterNTZ * $recall_enterNTZ) / ($precision_enterNTZ + $recall_enterNTZ))


$precision_exitNTZ = $true_positive_exitNTZ / ($true_positive_exitNTZ + $false_positive_exitNTZ) 
$recall_exitNTZ = $true_positive_exitNTZ  / ($true_positive_exitNTZ + $false_negative_exitNTZ) 
$accuracy_exitNTZ = ($true_positive_exitNTZ)/($true_positive_exitNTZ + $false_positive_exitNTZ + $false_negative_exitNTZ) 
$f_score_exitNTZ = (1+$f_beta) * (($precision_exitNTZ * $recall_exitNTZ) / ($precision_exitNTZ + $recall_exitNTZ))


$precision_park = $true_positive_park / ($true_positive_park + $false_positive_park) 
$recall_park = $true_positive_park / ($true_positive_park + $false_negative_park) 
$accuracy_park = ($true_positive_park)/($true_positive_park + $false_positive_park + $false_negative_park) 
$f_score_park = (1+$f_beta) * (($precision_park * $recall_park) / ($precision_park + $recall_park))


$precision_leave = $true_positive_leave / ($true_positive_leave + $false_positive_leave) 
$recall_leave = $true_positive_leave / ($true_positive_leave + $false_negative_leave) 
$accuracy_leave = ($true_positive_leave)/($true_positive_leave + $false_positive_leave + $false_negative_leave) 
$f_score_leave = (1+$f_beta) * (($precision_leave * $recall_leave) / ($precision_leave + $recall_leave))



####


#$error_rate = ($true_positive + $false_negative)/($true_positive + $false_positive + $false_negative) 


#"Error Rate :" + $($error_rate* 100) | out-file C:\powershell\output.txt -Append
"Precision: " + $($precision* 100) | out-file C:\powershell\output.txt -Append
"Recall : " + $($recall* 100) | out-file C:\powershell\output.txt -Append
"Accuracy : " + $($accuracy* 100) | out-file C:\powershell\output.txt -Append
"F score (beta=" + $($f_beta) + "): " + $($f_score* 100) | out-file C:\powershell\output.txt -Append


"" | out-file C:\powershell\output.txt -Append

"Individual Scores " | out-file C:\powershell\output.txt -Append
"------------------" | out-file C:\powershell\output.txt -Append



"ENTER STATE " | out-file C:\powershell\output.txt -Append
"True Positive: "  + $($true_positive_enter)  | out-file C:\powershell\output.txt -Append
"False Positive: " + $($false_positive_enter) | out-file C:\powershell\output.txt -Append
"False Negative: " + $($false_negative_enter) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append
"Precision: " + $($precision_enter* 100) | out-file C:\powershell\output.txt -Append
"Recall : " + $($recall_enter* 100) | out-file C:\powershell\output.txt -Append
"Accuracy : " + $($accuracy_enter* 100) | out-file C:\powershell\output.txt -Append
"F score (beta=" + $($f_beta) + "): " + $($f_score_enter* 100) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append





"EXIT STATE " | out-file C:\powershell\output.txt -Append


"True Positive: "  + $($true_positive_exit)  | out-file C:\powershell\output.txt -Append
"False Positive: " + $($false_positive_exit) | out-file C:\powershell\output.txt -Append
"False Negative: " + $($false_negative_exit) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append
"Precision: " + $($precision_exit* 100) | out-file C:\powershell\output.txt -Append
"Recall : " + $($recall_exit* 100) | out-file C:\powershell\output.txt -Append
"Accuracy : " + $($accuracy_exit* 100) | out-file C:\powershell\output.txt -Append
"F score (beta=" + $($f_beta) + "): " + $($f_score_exit* 100) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append





"ENTER NTZ STATE " | out-file C:\powershell\output.txt -Append

"True Positive: "  + $($true_positive_enterNTZ)  | out-file C:\powershell\output.txt -Append
"False Positive: " + $($false_positive_enterNTZ) | out-file C:\powershell\output.txt -Append
"False Negative: " + $($false_negative_enterNTZ) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append
"Precision: " + $($precision_enterNTZ* 100) | out-file C:\powershell\output.txt -Append
"Recall : " + $($recall_enterNTZ* 100) | out-file C:\powershell\output.txt -Append
"Accuracy : " + $($accuracy_enterNTZ* 100) | out-file C:\powershell\output.txt -Append
"F score (beta=" + $($f_beta) + "): " + $($f_score_enterNTZ* 100) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append



"EXIT NTZ STATE " | out-file C:\powershell\output.txt -Append

"True Positive: "  + $($true_positive_exitNTZ)  | out-file C:\powershell\output.txt -Append
"False Positive: " + $($false_positive_exitNTZ) | out-file C:\powershell\output.txt -Append
"False Negative: " + $($false_negative_exitNTZ) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append
"Precision: " + $($precision_exitNTZ* 100) | out-file C:\powershell\output.txt -Append
"Recall : " + $($recall_exitNTZ* 100) | out-file C:\powershell\output.txt -Append
"Accuracy : " + $($accuracy_exitNTZ* 100) | out-file C:\powershell\output.txt -Append
"F score (beta=" + $($f_beta) + "): " + $($f_score_exitNTZ* 100) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append



"PARK STATE " | out-file C:\powershell\output.txt -Append


"True Positive: "  + $($true_positive_park)  | out-file C:\powershell\output.txt -Append
"False Positive: " + $($false_positive_park) | out-file C:\powershell\output.txt -Append
"False Negative: " + $($false_negative_park) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append
"Precision: " + $($precision_park* 100) | out-file C:\powershell\output.txt -Append
"Recall : " + $($recall_park* 100) | out-file C:\powershell\output.txt -Append
"Accuracy : " + $($accuracy_park* 100) | out-file C:\powershell\output.txt -Append
"F score (beta=" + $($f_beta) + "): " + $($f_score_park* 100) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append




"LEAVE STATE " | out-file C:\powershell\output.txt -Append
"True Positive: "  + $($true_positive_leave)  | out-file C:\powershell\output.txt -Append
"False Positive: " + $($false_positive_leave) | out-file C:\powershell\output.txt -Append
"False Negative: " + $($false_negative_leave) | out-file C:\powershell\output.txt -Append

"" | out-file C:\powershell\output.txt -Append

"Precision: " + $($precision_leave* 100) | out-file C:\powershell\output.txt -Append
"Recall : " + $($recall_leave* 100) | out-file C:\powershell\output.txt -Append
"Accuracy : " + $($accuracy_leave* 100) | out-file C:\powershell\output.txt -Append
"F score (beta=" + $($f_beta) + "): " + $($f_score_leave* 100) | out-file C:\powershell\output.txt -Append




$time_window = $time_window - $time_dec
}
