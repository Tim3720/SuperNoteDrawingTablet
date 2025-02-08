import subprocess
import time

def move_mouse(x, y):
    """Move the mouse to (x, y) using dotool."""
    subprocess.run(["echo", "mousemove", str(x), str(y), "|", "dotool"])

def press_mouse():
    """Press and hold the left mouse button."""
    subprocess.run(["echo" , "mousedown BTN_LEFT", "|", "dotool"])

def release_mouse():
    """Release the left mouse button."""
    subprocess.run(["echo", "mouseup", "BTN_LEFT", "|", "dotool"])

def draw_stroke(points, delay=0.01):
    """Simulate drawing by moving the mouse through a series of points."""
    if not points:
        return

    # Move to the starting position
    move_mouse(*points[0])
    press_mouse()

    for x, y in points[1:]:
        move_mouse(x, y)
        time.sleep(delay)  # Small delay to simulate natural movement

    release_mouse()

# Example stroke (replace with real tablet coordinates)
time.sleep(3)
print("Start")
example_points = [(500, 500), (505, 505), (510, 510), (520, 520), (530, 530)]
draw_stroke(example_points, 0.1)

