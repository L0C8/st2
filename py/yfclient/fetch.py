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
    rsi: float | None = None


def compute_rsi(closes: list[float], period: int = 14) -> float | None:
    if period <= 0 or len(closes) < period + 1:
        return None

    gains: list[float] = []
    losses: list[float] = []
    for prev, curr in zip(closes, closes[1:]):
        delta = curr - prev
        gains.append(max(delta, 0.0))
        losses.append(max(-delta, 0.0))

    avg_gain = sum(gains[:period]) / period
    avg_loss = sum(losses[:period]) / period

    for gain, loss in zip(gains[period:], losses[period:]):
        avg_gain = ((avg_gain * (period - 1)) + gain) / period
        avg_loss = ((avg_loss * (period - 1)) + loss) / period

    if avg_loss == 0:
        return 100.0

    rs = avg_gain / avg_loss
    return 100.0 - (100.0 / (1.0 + rs))


def fetch_quote(symbol: str) -> Quote:
    try:
        import yfinance as yf
    except ImportError:
        print("Missing dependency: yfinance. Install with `pip install yfinance`.", file=sys.stderr)
        return Quote(symbol=symbol, currency=None, last_price=None, previous_close=None)

    ticker = yf.Ticker(symbol)
    info = getattr(ticker, "fast_info", None)
    currency = getattr(info, "currency", None) if info else None
    last_price = getattr(info, "last_price", None) if info else None
    prev_close = getattr(info, "previous_close", None) if info else None
    rsi_value: float | None = None

    try:
        history = ticker.history(period="6mo", interval="1d")
        closes = history.get("Close")
        if closes is not None:
            close_list = [float(x) for x in closes.dropna().tolist()]
            rsi_value = compute_rsi(close_list)
    except Exception as exc:  # noqa: BLE001
        print(f"Failed to compute RSI for {symbol}: {exc}", file=sys.stderr)

    return Quote(
        symbol=symbol,
        currency=currency,
        last_price=last_price,
        previous_close=prev_close,
        rsi=rsi_value,
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
