#!/bin/bash
export DISPLAY=:0
## For Q-learning
# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_one_link_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_1.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test_Q-learning.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test.sh"

# bash -x RL-MPTCP_test.sh /home/hong/result_figure/2017-07-18_15-44-58_q-learning2/q-table.csv std_qlearning2_all
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.82846

# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test_DQN.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test.sh"

## For Basic Experiment
cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_one_link_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test_DQN.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test.sh"
cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_DQN.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh"
cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_topo_1.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh"
cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_1.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh

bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_16-17-03_StdTopo100QueNoErrNoTraffic1LNet32Epis_test/my_final_model-29900.meta "std_all_DQN"
bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.82846

cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_topo_1.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh"


# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night

# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_1.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh

# bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_all_train_1"

# # mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
# mv /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc
# mv /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh



# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night
# # mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night

# # cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"

# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_0.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_train_errorlevel_0"
# sleep 1
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_1.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_train_errorlevel_1"
# sleep 1
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_2.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_train_errorlevel_2"
# sleep 1
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_3.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_train_errorlevel_3"
# sleep 1



# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night

# cp /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_topo_1.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# for (( j=0; j<2; j++ ))
# do
#   cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_${j}.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
  # cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_${j}.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
  # for (( i=0; i<3; i++ ))
  # do
  # cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_${i}.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh  
  # bash -x ./RL-MPTCP_test.sh  /home/hong/result_figure/2017-07-16_03-12-30_basic_10relu_trainlevel_2/my_final_model-29900.meta "basic_10relu_not10Mbps_app_${j}"
  # bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_16-17-03_StdTopo100QueNoErrNoTraffic1LNet32Epis_test/my_final_model-29900.meta "need_to_add"
  # bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_19-56-45_StdTopo100QueNoErrRdTrafficRdFailLink1LNet32Epis/my_final_model-26575.meta "std_linkfail_linkfailTrained_${j}"
  # sleep 1 
  # bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_linkfail_randomtrafficTrained_${j}"
  # sleep 1
  # bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_10relu_app_${j}_trainlevel_${i}"
  # done
# done

# mv /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
# mv /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh




# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night

# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_1.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"

# bash -x ./RL-MPTCP_test.sh  /home/hong/result_figure/2017-07-16_03-12-30_basic_10relu_trainlevel_2/my_final_model-29900.meta "std_all_1_1"
# sleep 1
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_16-17-03_StdTopo100QueNoErrNoTraffic1LNet32Epis_test/my_final_model-29900.meta "std_all_1_2"
# sleep 1
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_all_1_3"  

# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
# mv /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc
# mv /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh


######

# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night

# cp /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_topo_0.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# for (( i=0; i<3; i++ ))
# do
#   cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_${i}.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
#   bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "basic_10relu_trainlevel_${i}"
# done

# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh


# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night

# cp /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_topo_1.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# for (( j=0; j<4; j++ ))
# do
#   cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_${j}.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
#   for (( i=0; i<3; i++ ))
#   do
#     # cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_${i}.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
#     bash -x ./RL-MPTCP_test.sh  /home/hong/result_figure/2017-07-16_03-12-30_basic_10relu_trainlevel_2/my_final_model-29900.meta "std_10relu_app_${j}_trainlevel_${i}"
#     # bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_10relu_app_${j}_trainlevel_${i}"
#   done
# done

# mv /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh






###### Ver 0.4 About application rate

# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night

# for (( j=0; j<4; j++ ))
# do
#   cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_${j}.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
#   for (( i=0; i<3; i++ ))
#   do
#     bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "basic_otherTraffic_${j}_${i}"
#   done
# done

# mv /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc

######  Ver 0.3   About error rate and training method

# mv /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night

# cp /home/hong/workspace/mptcp/ns3/machineLearning/rl_server_1e-2.py /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py
# cp /home/hong/workspace/mptcp/ns3/machineLearning/RL_core_Adam.py /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py
# cp /home/hong/workspace/mptcp/ns3/RL-MPTCP_train_1.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
# for (( j=0; j<3; j++ ))
# do
#   for (( i=0; i<3; i++ ))
#   do
#     cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_${i}.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh"
#     bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "StdNormedOtherTrafficNoFailureAdam1e-2_trainLevel_1_errLevel_${i}_${j}"
#   done
# done

# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# mv /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py_backup_for_night /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py
# mv /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py_backup_for_night /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh



####     Ver 0.2

# mv /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night

# cp /home/hong/workspace/mptcp/ns3/machineLearning/rl_server_1e-2.py /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py
# cp /home/hong/workspace/mptcp/ns3/machineLearning/RL_core_Adam.py /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py
# for (( i=0; i<3; i++ ))
# do
#   cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_${i}.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh"
#   bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "StdNormedNoErrOtherTrafficNoFailureAdam1e-2_${i}"
# done

# cp /home/hong/workspace/mptcp/ns3/machineLearning/rl_server_1e-2.py /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py
# cp /home/hong/workspace/mptcp/ns3/machineLearning/RL_core_RMSProp.py /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py
# for (( i=0; i<3; i++ ))
# do
#   cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_${i}.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh"
#   bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "StdNormedNoErrOtherTrafficNoFailureRMSProp1e-2_${i}"
# done

# cp /home/hong/workspace/mptcp/ns3/machineLearning/rl_server_1e-3.py /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py
# cp /home/hong/workspace/mptcp/ns3/machineLearning/RL_core_Adam.py /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py
# for (( i=0; i<3; i++ ))
# do
#   cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_${i}.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh"
#   bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "StdNormedNoErrOtherTrafficNoFailureAdam1e-3_${i}"
# done

# cp /home/hong/workspace/mptcp/ns3/machineLearning/rl_server_1e-3.py /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py
# cp /home/hong/workspace/mptcp/ns3/machineLearning/RL_core_RMSProp.py /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py
# for (( i=0; i<3; i++ ))
# do
#   bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "StdNormedNoErrOtherTrafficNoFailureRMSProp1e-3_${i}"
# done

# mv /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py_backup_for_night /home/hong/workspace/mptcp/ns3/machineLearning/rl_server.py
# mv /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py_backup_for_night /home/hong/workspace/mptcp/ns3/machineLearning/RL_core.py
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh

####     Ver 0.1 

# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night

# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_no_err.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# for (( i=0; i<3; i++ ))
# do
#   bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "StdNormed2LayerSigmodNoErr${i}"
# done

# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_2e-6_err.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# for (( i=0; i<3; i++ ))
# do
#   bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "StdNormed2LayerSigmod2e-6Err${i}"
# done


# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
