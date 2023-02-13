import subprocess
import time
import random
import sys

def start(executable_file):
    return subprocess.Popen(
        executable_file,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)


def read(process):
    return process.stdout.readline().decode("utf-8").strip()


def write(process, message):
    process.stdin.write(f"{message.strip()}\n".encode("utf-8"))
    process.stdin.flush()


def terminate(process):
    process.stdin.close()
    process.terminate()
    process.wait(timeout=0.2)

# compares the quiescence search score with the static eval
def check_quiet(fen_string, process):
    write(process, "position fen " + fen_string)

    write(process, "eval")
    eval_score = read(process)

    write(process, "go quiescence")
    quiescence_score = read(process)

    return eval_score == quiescence_score

def main():
    if len(sys.argv) != 4:
        print("""
              Incorrect usage.\nPlease use 
              'python filter_quiet_positions.py input_file output_file uci_engine_command'.
        """)
        exit()

    lines = []
    with open(sys.argv[1], 'r') as f:
        lines = f.readlines()
    
    process = start(sys.argv[3])
    quiet_positions = []

    print('Checking positions...')
    for idx, line in enumerate(lines):
        fen = line.split('$')[0].strip()
        if check_quiet(fen, process):
            quiet_positions.append(line)
        if idx % 5000 == 0:
            print('Finished checking position ' + str(idx))

    print('Finished all ' + str(len(lines)) + ' positions, got ' + str(len(quiet_positions)) + ' quiet positions.')

    write(process, 'quit')
    terminate(process)

    random.shuffle(quiet_positions)
    with open(sys.argv[2], 'a') as f:
        for line in quiet_positions:
            f.write(line)

if __name__ == "__main__":
    main()