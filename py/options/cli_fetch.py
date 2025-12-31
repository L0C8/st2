from __future__ import annotations

import json
import pathlib
import sys

# Ensure the repo's py/ folder is on sys.path so `import options` works when run directly.
ROOT = pathlib.Path(__file__).resolve().parents[2]  # /home/.../st2
PY_DIR = ROOT / "py"
if str(PY_DIR) not in sys.path:
    sys.path.insert(0, str(PY_DIR))

from options.data import get_options_view_df


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print("Usage: cli_fetch.py <SYMBOL>", file=sys.stderr)
        return 1
    symbol = argv[1].strip().upper()
    straddle_df, stacked_df = get_options_view_df(symbol)
    if straddle_df is None and stacked_df is None:
        print(json.dumps({"symbol": symbol, "error": "no data"}, indent=2))
        return 2

    summary = {"symbol": symbol, "straddle_rows": 0, "stacked_rows": 0, "preview": {}}
    if straddle_df is not None:
        summary["straddle_rows"] = len(straddle_df)
        summary["preview"]["straddle_head"] = straddle_df.head(3).to_dict(orient="records")
    if stacked_df is not None:
        summary["stacked_rows"] = len(stacked_df)
        summary["preview"]["stacked_head"] = stacked_df.head(3).to_dict(orient="records")

    print(json.dumps(summary, indent=2, default=str))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
