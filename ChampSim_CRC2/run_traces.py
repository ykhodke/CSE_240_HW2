import re
import glob
from os import system
from os import listdir
from os.path import isfile, join
from os.path import exists
import shlex
import subprocess as sp

##list of replacement policies I am testing
#policies = ['lru-config1', 'lru-config2', 'ship_pp-config1', 'ship_pp-config2', 'multp-config1', 'multp-config2', 'lime-config1', 'lime-config2']

#experimental policies here
#policies = ['lime_10ts-config1', 'lime_10ts-config2', 'lime_14ts-config1', 'lime_14ts-config2', 'lime_18ts-config1', 'lime_18ts-config2', 'lime_22ts-config1', 'lime_22ts-config2', 'lime_26ts-config1', 'lime_26ts-config2', 'lime_30ts-config1', 'lime_30ts-config2']

#policies = ['lime_4ts-config1', 'lime_4ts-config2']

#policies = ['lime_ht_16-config1', 'lime_ht_16-config2', 'lime_ht_32-config1', 'lime_ht_32-config2', 'lime_ht_64-config1', 'lime_ht_64-config2', 'lime_ht_96-config1', 'lime_ht_96-config2', 'lime_ht_160-config1', 'lime_ht_160-config2', 'lime_ht_192-config1', 'lime_ht_192-config2' ]

#policies = ['red-config1', 'red-config2']

#policies = ['red_ad-config1', 'red_ad-config2',  'red_ah-config1', 'red_ah-config2',  'red_aq-config1', 'red_aq-config2',  'red_aqp-config1', 'red_aqp-config2',  'red_ld-config1', 'red_ld-config2',  'red_lh-config1', 'red_lh-config2',  'red_lq-config1', 'red_lq-config2',  'red_lqp-config1', 'red_lqp-config2']
#policies = ['lime_pc10-config2','lime_pc12-config2','lime_pc14-config2','lime_pc18-config2','lime_pc20-config2','lime_pc22-config2']
#policies = ['red_1_52-config2', 'red_2_60-config2', 'red_4_66-config2', 'red_5_69-config2']

policies = ['red_sw3-config2','red_sw4-config2','red_sw5-config2','red_259-config2','red_268-config2']

# Example of a run instruction
#./lru-config1 -warmup_instructions 1000000 -simulation_instructions 10000000 -traces trace/bzip2_10M.trace.gz

# Getting a list of all traces in the trace directory
mypath = '/datasets/home/56/156/ykhodke/CSE_240C/ChampSim_CRC2/traces'
all_traces = [f for f in listdir(mypath) if isfile(join(mypath, f))]


for policy in policies:
  process_list = []
  print("----- Running traces for policy {} ---- \n".format(policy))
  for number, trace in enumerate(all_traces):

    trace_name = re.search('(.+?).trace.gz', trace).group(1)
    my_result_file = "./results/"+policy+"-"+trace_name+".txt"
    print("\t -- Trace run results are beign stored in {} \n".format(my_result_file))
    
    my_inst = "./{} -warmup_instructions 1000000 -simulation_instructions 10000000 -traces traces/{}".format (policy, trace)
    args = shlex.split(my_inst)
    print(">>>> \n {} <<<".format(args))
    
    with open(my_result_file, "w") as res_file:
      process_list.append(sp.Popen(args, stdout=res_file))

  exit_codes = [p.wait() for p in process_list]
  print("\n****\n {} \n****\n".format(exit_codes))

print ("This run ended here")