import subprocess
import time

def getBestMove(fenPos, ms):
    return subprocess.run("./engine", input=f"ucinewgame\nposition fen {fenPos}\ngo movetime {ms}\nquit\n".encode(), capture_output=True).stdout.decode().split('\n')[-2].split()[1]


if __name__ == "__main__":
    with open('ERET.txt', 'r') as f:
        for line in f:
            fen = line[:line.index('bm')]
            # print(getBestMove(fen, 15000))
            try:
                print(subprocess.run("./engine", input=f"ucinewgame\nposition fen {fen}\ngo movetime {15000}\nquit\n".encode(), capture_output=True).stdout.decode().split('\n')[-2].split()[1])
            except:
                print(subprocess.run("./engine", input=f"ucinewgame\nposition fen {fen}\ngo movetime {15000}\nquit\n".encode(), capture_output=True).stdout)
