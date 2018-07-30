#!/usr/bin/env python

import os
import re
import sys
import glob
import subprocess
from optparse import OptionParser

HOME = os.path.dirname(os.path.realpath(__file__))
pathEnv=os.getenv('PATH')
os.environ['PATH']= "%s" %(HOME) + ":" + pathEnv 
dest_fps = 15
default_resolution = False
target_width = 0
target_height = 0
session_tag="new session started"

class Logger:
	def __init__(self, logfile):
		self.terminal = sys.stdout
		self.log = open(logfile, "a")
		sys.stdout = self
	def write(self, message):
		self.terminal.write(message)
		self.log.write(message) 

class AudioClip:
	def __init__(self):
		self.num = 0
		self.filename = []
		self.start_time = []
		self.end_time = []

        def update_audio_info(self, i, stime, etime):
		self.start_time[i] = stime
		self.end_time[i] = etime

	def put_file(self, name):
		if not (name in self.filename):
			self.filename.append(name)
			self.start_time.append(0.0)
			self.end_time.append(0.0)
			self.num = self.num + 1
		return self.filename.index(name)

	def max_length(self):
		return max(self.end_time)

	def print_filename(self):
		str = ""
		for i in range(self.num):
			if i > 0:
				len = self.start_time[i] - self.end_time[i-1]
			else:
				len = self.start_time[0]
			if len < 0.001:
				len = 0.001
			str = str + ("-f lavfi -t %.3f -i anullsrc=channel_layout=mono:sample_rate=48000 " % len)
			str = str + ("-i %s " % self.filename[i])
		return str

	def print_audio_info(self, i):
		print "Audio Clip %d: %s: start_time=%.3f, end_time=%.3f" % (i, self.filename[i], self.start_time[i], self.end_time[i])

	def print_ffmpeg(self, output_file):
		if self.num >= 1:
			str = "ffmpeg " + self.print_filename()
			str = str + "-filter_complex \"concat=n=%d:v=0:a=1[audio]\" " % (self.num * 2)
			str = str + " -map \"[audio]\" -to %f -y %s" % (self.max_length(), output_file)
		elif self.num == 1:
			str = "ffmpeg -i %s -c:a copy %s" % (self.filename[0], output_file)
		else:
			str = ""
		str = str + " 2>&1 | tee -a convert.log"
		return str

class VideoClip:
	def __init__(self):
		self.num = 0
		self.filename = []
		self.start_time = []
		self.end_time = []
		self.width = []
		self.height = []
		self.audio_file = ""
		self.audio_start_time = 0.0
		self.audio_end_time = 0.0
                self.max_width = 0
                self.max_height = 0


        def update_audio_info(self, audio_stime, audio_etime):
		self.audio_start_time = audio_stime
		self.audio_end_time = audio_etime

        def update_video_info(self, i, video_stime, video_etime):
		self.start_time[i] = video_stime
		self.end_time[i] = video_etime

	def put_file(self, name):
		if not (name in self.filename):
			self.filename.append(name)
			self.start_time.append(0.0)
			self.end_time.append(0.0)
			self.width.append(0)
			self.height.append(0)
			self.num = self.num + 1
		return self.filename.index(name)
	
	def max_resolution(self):
		self.max_width = max(self.width)
		self.max_height = max(self.height)
		return self.max_width, self.max_height

        def target_resolution(self):
                self.max_resolution()
                if default_resolution:
                    self.target_width = self.max_width
                    self.target_height = self.max_height
                else:
                    self.target_width = target_width
                    self.target_height = target_height
                return  self.target_width, self.target_height
	
	def max_length(self):
		return max(max(self.end_time), self.audio_end_time)
	
	def audio_delay_needed(self):
		return self.audio_file != "" and self.audio_start_time > 0.05
	def audio_apad_needed(self):
    	        return self.audio_file != "" and self.max_length() > self.audio_end_time
	def print_filter(self):
    	        str = "" 
		if self.audio_delay_needed():
		    audio_delay = int(self.audio_start_time*1000)
		    str = "[0]adelay=%d" % audio_delay
		    if self.audio_apad_needed():
    			str = str + ",apad"
		    str = str + "[audio];"
		elif self.audio_apad_needed():
    		    str = str + "[0]apad[audio];"
		source = "1"
		sink = "out2"
		for i in range(self.num):
			sink = "out%d" % (i+2)
			if i == self.num - 1:
				sink = "video"
			tmp = "[%d]scale=%dx%d,setpts=PTS-STARTPTS+%.3f/TB[scale%d];[%s][scale%d]overlay=eof_action=pass[%s];" % \
				( (i+2), self.target_width, self.target_height, self.start_time[i], (i+2), source, (i+2), sink )
			str = str + tmp
			source = sink
		return str[:-1]
	
	def print_filename(self):
		str = ""
		for i in range(self.num):
			str = str + ("-i %s " % self.filename[i])
		return str
	
	def print_ffmpeg(self, output_file):
		if self.audio_file == "":
			str = "ffmpeg -f lavfi -i anullsrc "
		else:
			str = "ffmpeg -i %s " % self.audio_file
		str = str + "-f lavfi -i \"color=black:s=%dx%d:r=15\" " % (self.target_width, self.target_height)
		str = str + self.print_filename()
		str = str + "-filter_complex \"%s\" " % self.print_filter()
		if self.audio_file == "":
			map_option = "-map \"[video]\""
		else:
			if self.audio_delay_needed() or self.audio_apad_needed():
				map_option = "-map \"[audio]\" -map \"[video]\" -c:a aac"
			else:
				map_option = "-map 0:a:0 -map \"[video]\" -c:a copy"
		str = str + " %s -c:v libx264 -r %d -preset veryfast -shortest -to %f -y %s" % (map_option, dest_fps, self.max_length(), output_file)
		str = str + " 2>&1 | tee -a convert.log"
		return str
	
	def print_audio_info(self):
		print "Audio Clip: %s: start_time=%.3f, end_time=%.3f" % (self.audio_file, self.audio_start_time, self.audio_end_time)
	
	def print_video_info(self, i):
		print "Video Clip %d: %s: start_time=%.3f, end_time=%.3f, width=%d, height=%d" % \
			(i, self.filename[i], self.start_time[i], self.end_time[i], self.width[i], self.height[i])
	
def filter_pat(f, pat):
    regex=re.compile(pat)
    lines=f.readlines()
    detected=False
    for line in lines:
        if regex.match(line):
            lines.remove(line)
            detected=True

    return (detected,lines)

def UidFileConvert(uid_file, suffix, option):	
	child_env = os.environ.copy()

	uid = os.path.splitext(uid_file)[0][4:]
	print "UID:"+uid
		
	clip = VideoClip()
	audio_clip = AudioClip()
	with open(uid_file) as f:
                av_1st_stime = [False]
                (hasSessionDetect, lines)=filter_pat(f, session_tag)
#                print("DEBUG:%s:%s" %(hasSessionDetect, lines))
		for line in lines:
			items = line.split(" ")
			#audio file
			if items[1][-3:] == "aac":
				index = audio_clip.put_file(items[1])
				if items[2] == "create":
                                    if not av_1st_stime[0]:
                                        av_1st_stime.append(float(items[0])) #mark it.

				    audio_clip.start_time[index] = float(items[0])
				elif items[2] == "close":
                                    audio_clip.end_time[index] = float(items[0])
                                
			#video file
			if items[1][-3:] == "mp4":
				index = clip.put_file(items[1])
				if items[2] == "create":
                                    if not av_1st_stime[0]:
                                        av_1st_stime.append(float(items[0])) #mark it.

                                    clip.start_time[index] = float(items[0])
				elif items[2] == "info":
					clip.start_time[index] = float(items[0])
					clip.width[index] = int(items[3][6:])
					clip.height[index] = int(items[4][7:])
					rotation = int(items[5][9:])
					if rotation == 90 or rotation == 270:
						clip.width[index], clip.height[index] = clip.height[index], clip.width[index]
				elif items[2] == "close":
					clip.end_time[index] = float(items[0])
			#video file
			if items[1][-4:] == "webm":
				index = clip.put_file(items[1])
				if items[2] == "create":
                                    if not av_1st_stime[0]:
                                        av_1st_stime.append(float(items[0])) #mark it.
                                    clip.start_time[index] = float(items[0])
				elif items[2] == "info":
				    clip.start_time[index] = float(items[0])
				    clip.width[index] = int(items[3][6:])
				    clip.height[index] = int(items[4][7:])
				    rotation = int(items[5][9:])
				    if rotation == 90 or rotation == 270:
				        clip.width[index], clip.height[index] = clip.height[index], clip.width[index]
				elif items[2] == "close":
					clip.end_time[index] = float(items[0])

                if not option:
                    clip.update_audio_info(clip.audio_start_time - av_1st_stime[1],
                            clip.audio_end_time - av_1st_stime[1])

                    for i in range(audio_clip.num):
                        audio_clip.update_audio_info(i, audio_clip.start_time[i] - av_1st_stime[1],
                                audio_clip.end_time[i] - av_1st_stime[1])

                    for i in range(clip.num):
                        clip.update_video_info(i, clip.start_time[i] - av_1st_stime[1],
                                clip.end_time[i] - av_1st_stime[1])

		clip.print_audio_info()
		for i in range(audio_clip.num):
			audio_clip.print_audio_info(i)
		for i in range(clip.num):
			clip.print_video_info(i)
			
	if audio_clip.num >= 1:
		print "Generate Audio File"
		tmp_audio = uid+"_tmp.m4a"
		command = audio_clip.print_ffmpeg(tmp_audio)
		clip.audio_file = tmp_audio
		clip.audio_start_time = 0.0
		clip.audio_end_time = audio_clip.max_length()
		print command
		subprocess.Popen(command, shell=True, env=child_env).wait()
	elif audio_clip.num == 1:
		clip.audio_file = audio_clip.filename[0]
		clip.audio_start_time = audio_clip.start_time[0]
		clip.audio_end_time = audio_clip.end_time[0]

	if clip.num > 0:
		print "Generate MP4 file:"
		print "Output resolution:", clip.target_resolution()
		output_file = uid + suffix + ".mp4"
		#print clip.print_filter()
		command =  clip.print_ffmpeg(output_file)
	else:
		tmp_audio = uid+"_tmp.m4a"
		output_file = uid+".m4a"
		if audio_clip.num >= 1:
			command = "mv %s %s" % (tmp_audio, output_file)
		elif audio_clip.num == 1:
			command = "ffmpeg -i %s -c:a copy -y %s" % (clip.audio_file, output_file)
	print command
	subprocess.Popen(command, shell=True, env=child_env).wait()
	print "\n\n"

	#remove tmp files
	os.system('rm -f *_tmp.m4a')
	return
	
class UserInfoPerMergedTxt:
	def __init__(self, ts, path):
                self.detect_start = False
		self.last_ts = ts
		self.path = path
        
        def detectStartedTxt():#detect 0.000
            return self.detect_start

                

class UserInfoMergedTxtDict:
    def __init__(self, uid, index, ts, path):
        self.uid = uid
        self.mergedTxtDict = dict()
        self.mergedTxtDict[index] = UserInfoPerMergedTxt(ts, path)

    def update(self, uid, index, ts, path):
            if uid != self.uid:
                print("Error:uid mismatch:%s:%s" %(uid, self.uid))

            if not self.mergedTxtDict.has_key(index):
                self.mergedTxtDict[index] = UserInfoPerMergedTxt(ts, path)
            else:
                print("Error:already has mergedTxtDict[%d]:%s" %(index, mergedTxtDict[index]))

    def hasTheMergedIndex(self, index):
        return self.mergedTxtDict.has_key(index)
                    

    def outputMergedUserInfoByIndex(self):
        for i in mergedTxtDict[index]:
            print("uid:%s, data:%s" %(self.uid, self.mergedTxtDict[i]))


def SessionConvert(folder_name, opt, saving):
	if not os.path.isdir(folder_name):
		print "Folder " + folder_name + " does not exit"
		return

	os.chdir(folder_name)
	os.system('rm -f *_merge.txt')
	all_uid_file = sorted(glob.glob("uid_*.txt"))
	if opt == 0:
		for uid_file in all_uid_file:
			UidFileConvert(uid_file, "_av", 0)
		#write a convert done file
		f = open("convert-done.txt", "w")
		f.close()
		return

	#merge
	dict_uid  = dict()
        merged_index = -1
	for uid_file in all_uid_file:
		uid = uid_file.split("_")[1]
		if not dict_uid.has_key(uid):
                        merged_index = 0
			dict_uid[uid] = UserInfoMergedTxtDict(uid, merged_index, 0.0, "uid_" + uid +"_"+ str(merged_index) + "_merge.txt")

		start_ts = -1.0
		last_ts = 0
		with open(uid_file) as f:
                        detectedOncePerTxt = False
                        (hasSessionDetect, lines)=filter_pat(f, session_tag)
                        print("DEBUG:%s:%s" %(hasSessionDetect, lines))

			for line in lines:
				items = line.split(" ")
                                if not detectedOncePerTxt and (hasSessionDetect or float(items[0]) == 0.000):
                                    detectedOncePerTxt = True

                                    if not dict_uid[uid].mergedTxtDict[merged_index].detect_start:
                                        dict_uid[uid].mergedTxtDict[merged_index].detect_start = True

                                    else: #already started,detect new one, so increase index
                                        merged_index += 1
                                        dict_uid[uid].update(uid, merged_index, 0.0, "uid_" + uid +"_"+ str(merged_index) + "_merge.txt")
                                       # dict_uid[uid].mergedTxtDict[merged_index]
                                

				if opt == 2 and items[1].split(".")[1] != "aac": #aac
					continue
				elif opt == 3 and items[1].split(".")[1] == "aac": #mp4
					continue
				if start_ts < 0:
					if float(items[0]) > 0:
						start_ts = float(items[0])
					else:
						start_ts = 0
				if saving:
					items[0] = "%.3f" % (float(items[0]) - start_ts + dict_uid[uid].mergedTxtDict[merged_index].last_ts) 
				else:
					items[0] = "%.3f" % (float(items[0])) 
				last_ts = float(items[0])
				file = open(dict_uid[uid].mergedTxtDict[merged_index].path, "a")
				file.write(' '.join(items))
				file.close()
			dict_uid[uid].mergedTxtDict[merged_index].last_ts = last_ts + 0.1
	
	all_merge_file = sorted(glob.glob("*_merge.txt"))
	for merge_file in all_merge_file:
		if opt == 1:
			UidFileConvert(merge_file, "_av", opt)
		else:
			UidFileConvert(merge_file, "", opt)
		
	os.system('rm -f *_merge.txt')

	#write a convert done file
	f = open("convert-done.txt", "w")
	f.close()
	return	

parser = OptionParser()
parser.add_option("-f", "--folder", type="string", dest="folder", help = "Convert folder", default = "")
parser.add_option("-m", "--mode", type="int", dest="mode", help = "Convert merge mode, \
[0: txt merge A/V(Default); 1: uid merge A/V; 2: uid merge audio; 3: uid merge video]", default = 0)
parser.add_option("-p", "--fps", type="int", dest="fps", help = "Convert fps, default 15", default = 15)
parser.add_option("-s", "--saving", action="store_true", dest="saving", help = "Convert Do not time sync", default = False)
parser.add_option("-r", "--resolution", type="int", dest="resolution", nargs=2, help = "Specific resolution to convert '-r width height' \nEg:'-r 640 360'", default=(0,0))

(options, args) = parser.parse_args()
if not options.folder:
        parser.print_help()
	parser.error("Not set folder")
if options.mode < 0 or options.mode > 3:
	parser.error("Invalid mode")
if options.fps <= 0:
	parser.error("Invalid fps")

if options.resolution[0] < 0 or options.resolution[1] < 0:
    parser.error("Invalid resolution width/height")
elif options.resolution[0] == 0 and options.resolution[1] == 0:
    default_resolution = True
else:
    target_width = options.resolution[0]
    target_height = options.resolution[1]

os.system("rm -f " + options.folder + "/convert.log")
Logger(options.folder + "/convert.log")

if options.fps < 5:
	print "fps < 5, set to 5"
	dest_fps = 5
elif options.fps > 120:
	print "fps > 120, set to 120"
	dest_fps = 120
else:
	dest_fps = options.fps

SessionConvert(options.folder, options.mode, options.saving)

