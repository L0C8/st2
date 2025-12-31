from __future__ import annotations

import json
import sys
from dataclasses import asdict, dataclass


@dataclass
class Quote:
    symbol: str
    currency: str | None
    last_price: float | None
    previous_close: float | None


def fetch_quote(symbol: str) -> Quote:
    try:
        import yfinance as yf
    except ImportError:
        print("Missing dependency: yfinance. Install with `pip install yfinance`.", file=sys.stderr)
        return Quote(symbol=symbol, currency=None, last_price=None, previous_close=None)

    ticker = yf.Ticker(symbol)
    info = getattr(ticker, "fast_info", None)
    currency = None
    last_price = None
    prev_close = None
    if info:
        currency = getattr(info, "currency", None)
        last_price = getattr(info, "last_price", None)
        prev_close = getattr(info, "previous_close", None)
    return Quote(
        symbol=symbol,
        currency=currency,
        last_price=last_price,
        previous_close=prev_close,
    )


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print("Usage: fetch.py <SYMBOL>", file=sys.stderr)
        return 1
    symbol = argv[1].strip().upper()
    quote = fetch_quote(symbol)
    print(json.dumps(asdict(quote), indent=2, sort_keys=True, default=str))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
