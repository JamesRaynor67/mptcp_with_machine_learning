#!/bin/bash
# export DISPLAY=:0

# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-20_21-30-19_std_all_train_1_ActorCritic_0_test" 0.82846
# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-20_23-12-24_std_all_train_1_ActorCritic_1_test" 0.82846
# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-21_00-53-07_std_all_train_1_ActorCritic_2_test" 0.82846
# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-21_01-14-46_std_application_linkfail_1_test" 0.9694
# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-21_01-37-44_std_errorlevel_1_all_links_errer_test" 0.997
# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-21_01-54-32_std_errorlevel_2_all_links_errer_test" 0.99
# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-21_02-08-22_std_errorlevel_3_all_links_errer_test" 0.97
# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-21_12-01-00_std_qlearning2_all_test" 0.82846
# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-21_12-28-04_basic_train_0_test" 1
# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-21_12-41-43_basic_train_1_test" 1
# bash -x RL-MPTCP_generate_thesis_figure.sh "/home/hong/result_figure/2017-07-21_12-55-22_basic_train_2_test" 1




cp "/home/hong/workspace/mptcp/ns3/mptcp-scheduler-largest-dbp.cc_backup" "/home/hong/workspace/mptcp/ns3/src/internet/model/mptcp-scheduler-largest-dbp.cc"
cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_one_link_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_0.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"

cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_DQN.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh"
cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test_DQN.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test.sh"

cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_1_special.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
for (( i=0; i<3; i++ ))
do
bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_all_train_DQN_application_1_special_${i}"
bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.91667 #  (1.0/3 * (300.0/(300+50*2)) + 2.0/3)
done

cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_2_special.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
for (( i=0; i<3; i++ ))
do
bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_all_train_DQN_application_2_special_${i}"
bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.83333 # (1.0/3 * (300.0/(300+150*2)) + 2.0/3)
done

cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_3_special.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
for (( i=0; i<3; i++ ))
do
bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_all_train_DQN_application_3_special_${i}"
bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.77778 # (1.0/3 * (300.0/(300+300*2)) + 2.0/3)
done

# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_AC.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test_AC.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test.sh"
# bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_all_train_1_ActorCritic_application_2_special"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.82846

# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test_Q-learning.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test.sh"

# bash -x RL-MPTCP_test.sh /home/hong/result_figure/2017-07-18_15-44-58_q-learning2/q-table.csv "std_qlearning2_all_application_2_special"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.82846

cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_DQN.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh"
cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test_DQN.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test.sh"


# cp "/home/hong/workspace/mptcp/ns3/mptcp-scheduler-largest-available_buffer.cc_backup" "/home/hong/workspace/mptcp/ns3/src/internet/model/mptcp-scheduler-largest-dbp.cc"
# # cp "/home/hong/workspace/mptcp/ns3/mptcp-scheduler-largest-dbp.cc_backup" "/home/hong/workspace/mptcp/ns3/src/internet/model/mptcp-scheduler-largest-dbp.cc"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_one_link_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_1.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_DQN.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test_DQN.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_test.sh"

# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_2.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh"
# bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_train_errorlevel_2_DQN_AB"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.99

# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_3.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh"
# bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_train_errorlevel_3_DQN_AB"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.97

# # cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_2.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh"
# # bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_all_train_1_but_errorlevel_2_DQN_AB"
# # bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.82846

# # cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_3.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh"
# # bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_all_train_1_but_errorlevel_3_DQN_AB"
# # bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.82846

# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_0.sh" "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-scheduler-largest-dbp.cc_backup" "/home/hong/workspace/mptcp/ns3/src/internet/model/mptcp-scheduler-largest-dbp.cc"

#####

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_one_link_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_0.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"

# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_application_linkfail_1"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 1


# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_all_links_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_1.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_errorlevel_1_all_links_errer"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.997

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_all_links_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_2.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_errorlevel_2_all_links_errer"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.99

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_all_links_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_3.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_errorlevel_3_all_links_errer"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.97




# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_one_link_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_1.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_errorlevel_1_one_link_errer"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.997

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_one_link_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_2.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_errorlevel_2_one_link_errer"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.99

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_one_link_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_3.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_errorlevel_3_one_link_errer"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.97



# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-topology_one_link_error.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-topology.cc"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_0.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_application_level_0"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 1

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_application_level_1"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.85714

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_2.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_application_level_2"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.66667

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_3.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_application_level_3"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.5

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_1_30s.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_application_level_1_30s"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.85714

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_2_30s.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_application_level_2_30s"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.66667

# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_3_30s.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_application_level_3_30s"
# bash -x RL-MPTCP_generate_thesis_figure.sh $(ls -td /home/hong/result_figure/*/ | head -1) 0.5

# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night

# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
# cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_1.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh

# # bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_all_train_1_ActorCritic"
# bash -x RL-MPTCP_test.sh /home/hong/result_figure/2017-07-18_15-44-58_q-learning2/q-table.csv std_qlearning2_all

# # mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
# mv /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc
# mv /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh

# mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night

# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_0.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_19-56-45_StdTopo100QueNoErrRdTrafficRdFailLink1LNet32Epis/my_final_model-26575.meta "std_linkfail_linkfailTrained_0"
# cp "/home/hong/workspace/mptcp/ns3/mptcp-test_linkfail_1.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc"
# bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_19-56-45_StdTopo100QueNoErrRdTrafficRdFailLink1LNet32Epis/my_final_model-26575.meta "std_linkfail_linkfailTrained_1"

# mv /home/hong/workspace/mptcp/ns3/mptcp-test.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-test.cc


# # mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night
# # mv /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night

# # cp /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_topo_1.sh /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
# for (( j=0; j<4; j++ ))
# do
#   # cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters_errLevel_${j}.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh
#   cp "/home/hong/workspace/mptcp/ns3/mptcp-helper-application_${j}.cc" "/home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc"
#   # cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_${j}.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
#   for (( i=0; i<3; i++ ))
#   do
#   cp "/home/hong/workspace/mptcp/ns3/RL-MPTCP_train_${i}.sh" /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh  
#   # bash -x ./RL-MPTCP_test.sh  /home/hong/result_figure/2017-07-16_03-12-30_basic_10relu_trainlevel_2/my_final_model-29900.meta "basic_10relu_not10Mbps_app_${j}"
#   # bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_16-17-03_StdTopo100QueNoErrNoTraffic1LNet32Epis_test/my_final_model-29900.meta "need_to_add"
#   # bash -x ./RL-MPTCP_test.sh /home/hong/result_figure/2017-06-29_17-22-12_StdTopo100QueNoErrRdTraffic1LNet32Epis/my_final_model-28273.meta "std_10relu_not10Mbps_errorlevel_${j}"  
#   bash -x /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh "std_10relu_app_${j}_trainlevel_${i}"
#   done
# done

# # mv /home/hong/workspace/mptcp/ns3/mptcp-helper-application.cc_backup_for_night /home/hong/workspace/mptcp/ns3/scratch/run/mptcp-helper-application.cc
# mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_train.sh
# # mv /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh_backup_for_night /home/hong/workspace/mptcp/ns3/RL-MPTCP_experiment_parameters.sh

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
