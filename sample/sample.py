import subprocess
import sys
import textwrap
import threading
import time

import win32session


FINALIZATION_CASE = textwrap.dedent(
    """
    import win32session

    class Callback:
        def __call__(self):
            pass

    # Deliberately omit off(). The module-owned state must release this callback
    # before CPython tears down its thread state and the GIL.
    win32session.set(Callback())
    """
)


def test_interpreter_finalization(iterations=10):
    for iteration in range(iterations):
        result = subprocess.run(
            [sys.executable, "-c", FINALIZATION_CASE],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
        )

        if result.returncode != 0:
            raise RuntimeError(
                "finalization subprocess {} failed with exit code {}:\n{}".format(
                    iteration + 1,
                    result.returncode,
                    result.stderr,
                )
            )


def test_normal_lifecycle(iterations=5):
    for iteration in range(iterations):
        result = []

        win32session.set(lambda: None)

        worker = threading.Thread(target=lambda: result.append(win32session.run()))
        worker.start()

        deadline = time.time() + 5

        while worker.is_alive() and time.time() < deadline:
            win32session.off()
            worker.join(0.01)

        if worker.is_alive():
            raise RuntimeError(
                "session worker {} did not stop".format(iteration + 1)
            )

        if result != [True]:
            raise RuntimeError(
                "session worker {} failed: {!r}".format(iteration + 1, result)
            )


if __name__ == "__main__":
    print("sys.version: {}".format(sys.version))
    print("call win32session.off: {}".format(win32session.off()))

    test_normal_lifecycle()

    print("normal lifecycle: 5/5 clean exits")

    test_interpreter_finalization()

    print("interpreter finalization: 10/10 clean exits")
