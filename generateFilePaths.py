import os
import sys


path_dataset = sys.argv[1]
filename_out = sys.argv[2]

sys.stdout = open(filename_out, 'w')

for type_file in os.listdir(path_dataset):
	for label in os.listdir(path_dataset + '/' + type_file):
		count = 0
		for video in os.listdir(path_dataset + '/' + type_file + '/' + label):
			print (path_dataset + '/' + type_file + '/' + label + '/' + video + ' ' + str(count) +' sys.stdout' )
			count +=1
