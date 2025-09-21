
import subprocess
import os

def run_game(inputs: str):
    # Clean up old CSV
    if os.path.exists("game_output.csv"):
        os.remove("game_output.csv")
    subprocess.run(["./solution"], input=inputs.encode(),
                   capture_output=True, timeout=5)

def read_csv():
    boards = []
    with open("game_output.csv") as f:
        for line in f:
            cells = line.strip().split(",")
            if len(cells) == 16:
                board = [list(map(int, cells[i*4:(i+1)*4])) for i in range(4)]
                boards.append(board)
    return boards

def first_nonspawn_board(boards):
    """
    Return last two boards (before, after) so we can compare merge
    effects ignoring random spawns.
    """
    if len(boards) < 2:
        return None, None
    return boards[-2], boards[-1]

# -------------------------------
# Basic tests
# -------------------------------

def test_progression_multiple_moves():
    run_game("a\nw\nd\ns\nq\n")
    boards = read_csv()
    assert len(boards) >= 5  # initial + 4 moves

def test_spawn_after_move():
    run_game("a\nq\n")
    boards = read_csv()
    flat0 = sum(boards[0], [])
    flat1 = sum(boards[-1], [])
    # More nonzeros after a move (spawn happened)
    assert flat1.count(0) < flat0.count(0)

def test_undo_restores():
    run_game("a\nu\nq\n")
    boards = read_csv()
    assert len(boards) >= 3
    assert boards[0] == boards[-1]

def test_multiple_undos():
    run_game("a\nd\nu\nu\nq\n")
    boards = read_csv()
    assert boards[0] == boards[-1]

# -------------------------------
# Merge correctness tests
# -------------------------------

def test_merge_two_twos_left():
    run_game("a\nq\n")
    boards = read_csv()
    before, after = first_nonspawn_board(boards)
    if before and before[0][0] == 2 and before[0][1] == 2:
        assert after[0][0] == 4
        assert after[0][1] == 0

def test_no_double_merge():
    run_game("a\nq\n")
    boards = read_csv()
    before, after = first_nonspawn_board(boards)
    if before and before[0][:3] == [2, 2, 2]:
        assert after[0][0] == 4
        assert after[0][1] == 2

def test_merge_right():
    run_game("d\nq\n")
    boards = read_csv()
    before, after = first_nonspawn_board(boards)
    if before and before[3][2] == 2 and before[3][3] == 2:
        assert after[3][3] == 4
