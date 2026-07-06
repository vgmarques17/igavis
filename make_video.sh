#!/bin/bash
#SBATCH --time=0:30:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=288
#SBATCH --ntasks-per-core=1
#SBATCH --gpus-per-task=0
#SBATCH --signal=TERM\@5
#SBATCH --hint=nomultithread
#SBATCH --account=lp157
## SBATCH --partition=debug
#SBATCH --uenv=prgenv-gnu/25.11:v1
#SBATCH --view=default,modules
#SBATCH --output=slurm-%x.%A_%a.out

# sbatch --job-name make_video --array 6-10 ./make_video.sh

ml python
source "/users/vgonalve/exec/igavis/igavis_env/bin/activate"
unset PYTHONPATH


BASE=~/exec/isolation-kit/pipeline/4_induce


printf -v ITER "%d" $SLURM_ARRAY_TASK_ID 
# ITER=2 #FIXME!!!
MODELS=(
    "pt_000" 
    "pt_001" 
    "pt_002" 
    "pt_003" 
    "pt_004" 
    "pt_005" 
    "pt_006" 
    "pt_007" 
    "pt_008" 
    "pt_009" 
    "pt_010"
    "pt_011"
    "pt_012"
    "pt_013"
    "pt_014"
    "pt_015"
    "pt_016"
    "pt_017"
    "pt_018"
    "pt_019"
    "pt_020"
    "pt_021"
    "pt_022"
)
pt_id=${MODELS[$ITER]}

results_folder=$(ls -d1 ${BASE}/results/${pt_id}* | sort -r | head -1)
echo "Reading simulations from ${results_folder}"

output_dirs=$(ls -d1 ${results_folder}/point_*/)

Xvfb :99 -screen 0 1024x768x24 >/dev/null 2>&1 & # or whatever display number instead of 99
xvfb_pid=$!

export DISPLAY=:99
pids=()
for output_dir in $output_dirs; do
    point_id=$(basename "$output_dir" | cut -d'_' -f2)
    echo "Processing $pt_id, point $point_id"
    mkdir ~/exec/igavis/anims/${pt_id}_anim_${point_id}
    igavis $BASE/data/${pt_id}/LA_RA_bilayer_with_fiber_um.vtk $BASE/results/${pt_id}*/point_${point_id}/vm.igb\
     --camera-config isolation_patient_cameras.json --camera-preset ${pt_id}\
     --np 10 --solid-val -1 --transp-val 200 --output-path ~/exec/igavis/anims/${pt_id}_anim_${point_id}&
    pids+=($!)
done

wait "${pids[@]}"

for output_dir in $output_dirs; do
    point_id=$(basename "$output_dir" | cut -d'_' -f2)
    cd ~/exec/igavis/anims/${pt_id}_anim_${point_id}
    ffmpeg -framerate 25 -pattern_type glob -i 'vm*.png' -c:v libx264 -crf 14 ${pt_id}_${point_id}.mp4
done

killall Xvfb