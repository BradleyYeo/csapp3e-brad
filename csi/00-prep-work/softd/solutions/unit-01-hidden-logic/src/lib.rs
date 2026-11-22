//! Unit 1 Reference Solution: RateLimiter with explicit Hoare contracts and invariants.

/// Token-bucket rate limiter preserving strong state invariants across all operations.
///
/// # Struct Invariant
/// - `self.tokens <= self.capacity` holds before and after every public method call.
/// - `self.last_refill_ms` never moves backwards.
/// - `self.usage_watermark` is monotonically non-decreasing.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct RateLimiter {
    capacity: u64,
    tokens: u64,
    refill_per_sec: u64,
    last_refill_ms: u64,
    usage_watermark: u64,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum LimitError {
    InsufficientTokens { available: u64, requested: u64 },
    ExceedsCapacity { capacity: u64, requested: u64 },
}

impl RateLimiter {
    /// Creates a full bucket at `start_ms`.
    ///
    /// # Postcondition
    /// - `tokens() == capacity` and struct invariants hold.
    pub fn new(capacity: u64, refill_per_sec: u64, start_ms: u64) -> Self {
        let rl = Self {
            capacity,
            tokens: capacity,
            refill_per_sec,
            last_refill_ms: start_ms,
            usage_watermark: 0,
        };
        rl.assert_invariants();
        rl
    }

    pub fn capacity(&self) -> u64 {
        self.capacity
    }

    pub fn tokens(&self) -> u64 {
        self.tokens
    }

    pub fn last_refill_ms(&self) -> u64 {
        self.last_refill_ms
    }

    fn assert_invariants(&self) {
        debug_assert!(self.tokens <= self.capacity, "tokens must never exceed capacity");
    }

    /// Refills elapsed tokens since `last_refill_ms` and attempts to consume `cost`.
    ///
    /// # Precondition
    /// - Arbitrary `now_ms`; non-monotonic timestamps (`now_ms < last_refill_ms`) are safely treated
    ///   as 0 elapsed time without rewinding `last_refill_ms`.
    ///
    /// # Postcondition
    /// - If `cost > capacity`, returns `Err(ExceedsCapacity)` with `tokens` unchanged (after any valid refill).
    /// - If available tokens `< cost`, returns `Err(InsufficientTokens)` with `tokens` unconsumed.
    /// - Otherwise consumes `cost` tokens and preserves `tokens <= capacity`.
    pub fn refill_and_take(&mut self, now_ms: u64, cost: u64) -> Result<u64, LimitError> {
        self.assert_invariants();

        if now_ms > self.last_refill_ms {
            let elapsed_ms = now_ms - self.last_refill_ms;
            let added = (elapsed_ms as u128 * self.refill_per_sec as u128) / 1000;
            if added > 0 {
                let capped = (self.tokens as u128 + added).min(self.capacity as u128) as u64;
                self.tokens = capped;
                // Advance clock only by whole tokens generated to prevent sub-token drift when needed,
                // or clamp cleanly to `now_ms` when `added > 0`.
                self.last_refill_ms = now_ms;
            }
        }

        if cost > self.capacity {
            self.assert_invariants();
            return Err(LimitError::ExceedsCapacity {
                capacity: self.capacity,
                requested: cost,
            });
        }

        if self.tokens < cost {
            self.assert_invariants();
            return Err(LimitError::InsufficientTokens {
                available: self.tokens,
                requested: cost,
            });
        }

        self.tokens -= cost;
        self.assert_invariants();
        Ok(self.tokens)
    }

    /// Credits bonus tokens back into the bucket without exceeding `capacity`.
    ///
    /// # Postcondition
    /// - `tokens' == min(capacity, tokens + amount)`.
    pub fn credit_tokens(&mut self, amount: u64) {
        self.assert_invariants();
        self.tokens = self.tokens.saturating_add(amount).min(self.capacity);
        self.assert_invariants();
    }

    /// Atomically reserves every cost in `costs`.
    ///
    /// # Postcondition
    /// - On `Ok(rem)`: `rem == tokens_before - sum(costs)` and `sum(costs) <= tokens_before`.
    /// - On `Err(_)`: `self.tokens` is strictly identical to `tokens_before` (no partial drain).
    pub fn reserve_batch(&mut self, costs: &[u64]) -> Result<u64, LimitError> {
        self.assert_invariants();

        let mut total: u64 = 0;
        for &cost in costs {
            if cost > self.capacity {
                return Err(LimitError::ExceedsCapacity {
                    capacity: self.capacity,
                    requested: cost,
                });
            }
            total = total.saturating_add(cost);
        }

        if total > self.tokens {
            return Err(LimitError::InsufficientTokens {
                available: self.tokens,
                requested: total,
            });
        }

        self.tokens -= total;
        self.assert_invariants();
        Ok(self.tokens)
    }

    /// Returns minimum milliseconds needed before `refill_and_take(now + wait, cost)` succeeds.
    ///
    /// # Postcondition
    /// - When `Ok(wait)` and `wait < u64::MAX`, calling `refill_and_take(self.last_refill_ms + wait, cost)`
    ///   with no intervening drains is guaranteed to return `Ok(_)`.
    pub fn effective_wait_ms(&self, cost: u64) -> Result<u64, LimitError> {
        self.assert_invariants();
        if cost > self.capacity {
            return Err(LimitError::ExceedsCapacity {
                capacity: self.capacity,
                requested: cost,
            });
        }
        if self.tokens >= cost {
            return Ok(0);
        }
        if self.refill_per_sec == 0 {
            return Ok(u64::MAX);
        }
        let deficit = (cost - self.tokens) as u128;
        let rate = self.refill_per_sec as u128;
        let millis = (deficit * 1000).div_ceil(rate);
        Ok(u64::try_from(millis).unwrap_or(u64::MAX))
    }

    /// Reconciles an external monotonically increasing cumulative usage report.
    ///
    /// # Postcondition
    /// - Stale reports (`reported_total <= usage_watermark`) are no-ops returning `0`.
    /// - Otherwise deducts `reported_total - old_watermark` (clamped at 0) and raises watermark.
    pub fn reconcile_usage(&mut self, reported_total: u64) -> u64 {
        self.assert_invariants();
        if reported_total <= self.usage_watermark {
            return 0;
        }
        let delta = reported_total - self.usage_watermark;
        self.tokens = self.tokens.saturating_sub(delta);
        self.usage_watermark = reported_total;
        self.assert_invariants();
        delta
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn basic_take_and_refill_works() {
        let mut rl = RateLimiter::new(10, 5, 1_000);
        assert_eq!(rl.refill_and_take(1_000, 6), Ok(4));
        assert_eq!(rl.refill_and_take(1_400, 3), Ok(3));
    }

    #[test]
    fn happy_path_batch_works() {
        let mut rl = RateLimiter::new(10, 1, 0);
        assert_eq!(rl.reserve_batch(&[3, 4, 2]), Ok(1));
    }

    #[test]
    fn monotonic_usage_reconcile_works() {
        let mut rl = RateLimiter::new(20, 0, 0);
        assert_eq!(rl.reconcile_usage(5), 5);
        assert_eq!(rl.reconcile_usage(9), 4);
        assert_eq!(rl.tokens(), 11);
    }

    #[test]
    fn batch_failure_leaves_tokens_unchanged() {
        let mut rl = RateLimiter::new(10, 0, 0);
        let err = rl.reserve_batch(&[4, 4, 5]).unwrap_err();
        assert_eq!(
            err,
            LimitError::InsufficientTokens {
                available: 10,
                requested: 13
            }
        );
        assert_eq!(rl.tokens(), 10);
    }

    #[test]
    fn credit_tokens_preserves_capacity_invariant() {
        let mut rl = RateLimiter::new(10, 0, 0);
        rl.refill_and_take(0, 3).unwrap();
        rl.credit_tokens(20);
        assert!(rl.tokens() <= rl.capacity());
        assert_eq!(rl.tokens(), 10);
    }

    #[test]
    fn non_monotonic_timestamp_does_not_corrupt_clock() {
        let mut rl = RateLimiter::new(10, 10, 5_000);
        rl.refill_and_take(5_000, 10).unwrap();
        assert!(rl.refill_and_take(2_000, 1).is_err());
        assert_eq!(rl.last_refill_ms(), 5_000);
        assert_eq!(rl.refill_and_take(5_100, 1), Ok(0));
    }

    #[test]
    fn wait_duration_guarantees_subsequent_take_succeeds() {
        let mut rl = RateLimiter::new(10, 3, 1_000);
        rl.refill_and_take(1_000, 10).unwrap();
        let wait = rl.effective_wait_ms(2).unwrap();
        assert!(rl.refill_and_take(1_000 + wait, 2).is_ok());
    }

    #[test]
    fn stale_reconcile_report_does_not_infect_future_charges() {
        let mut rl = RateLimiter::new(20, 0, 0);
        assert_eq!(rl.reconcile_usage(10), 10);
        assert_eq!(rl.reconcile_usage(4), 0);
        assert_eq!(rl.reconcile_usage(12), 2);
        assert_eq!(rl.tokens(), 8);
    }
}
