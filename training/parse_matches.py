import chess.pgn
import io
import random
import sys

# outputs the fens from a match, along with the score (separated by $)
def output_fen(match):
    pgn = io.StringIO(match)
    game = chess.pgn.read_game(pgn)

    ans = []
    outcome = match.split()[-1]

    if outcome not in ["1-0", "0-1", "1/2-1/2"]:
        print("Outcome Error")
        print(match)
        exit()

    while game.next():
        game = game.next()
        ans.append(game.board().fen() + " $" + outcome)
    
    return ans

def main():
    if len(sys.argv) != 3:
        print("Incorrect usage.\nPlease use 'python parse_matches.py input_file output_file'.")
        exit()

    lines = []
    num = 0
    total = 0
    match = ""
    matches = []

    # get the matches from the input file
    with open(sys.argv[1], 'r') as f:
        lines = f.readlines()

    # get total number of matches
    for line in lines:
        if line == "[Event \"?\"]\n":
            total += 1

    # loop through every match in the file
    # get the fens, shuffle them, and append them to the list
    try:
        for line in lines:
            if line == "[Event \"?\"]\n":
                if len(match):
                    curr_match = output_fen(match)
                    random.shuffle(curr_match)
                    matches += curr_match

                    num += 1
                    match = ""
                    print("Match " + str(num) + "/" + str(total) + " parsed.")
            match += line
    except KeyboardInterrupt:
        pass

    # at the end the list is shuffled again
    random.shuffle(matches)

    # write the output
    with open(sys.argv[2], 'a') as f:
        for line in matches:
            f.write(line + '\n')


if __name__ == "__main__":
    main()
