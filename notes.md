# scirpt used to break video into files
ffmpeg ...

# run through c program
sh build.sh && ./main <dir_containing_images_files>

# script use to re assembly files into video again
 ffmpeg -framerate 60 -start_number 1 -i output_renamed/frames%06d.png -c:v libx264 -pix_fmt yuv420p video/output.mkv
