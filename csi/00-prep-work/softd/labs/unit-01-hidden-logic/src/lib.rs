//! Unit 1 Starter: Token-bucket rate limiter and usage reconciler.
//!
//! Passes `baseline_tests`, yet violates modular reasoning and Hoare postconditions.

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
    pub fn new(capacity: u64, refill_per_sec: u64, start_ms: u64) -> Self {
        Self {
            capacity,
            tokens: capacity,
            refill_per_sec,
            last_refill_ms: start_ms,
            usage_watermark: 0,
        }
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

    /// Refills elapsed tokens since `last_refill_ms` and attempts to consume `cost`.
    pub fn refill_and_take(&mut self, now_ms: u64, cost: u64) -> Result<u64, LimitError> {
        if cost > self.capacity {
            return Err(LimitError::ExceedsCapacity {
                capacity: self.capacity,
                requested: cost,
            });
        }

        // FLAW (Unit 1 - Hidden temporal precondition):
        // When `now_ms < self.last_refill_ms`, `saturating_sub` yields 0 elapsed ms,
        // yet `self.last_refill_ms` is still overwritten backwards in time.
        let elapsed_ms = now_ms.saturating_sub(self.last_refill_ms);
        let added = (elapsed_ms * self.refill_per_sec) / 1000;
        if added > 0 {
            self.tokens = (self.tokens + added).min(self.capacity);
            self.last_refill_ms = now_ms;
        } else if now_ms < self.last_refill_ms {
            self.last_refill_ms = now_ms;
        }

        if self.tokens < cost {
            return Err(LimitError::InsufficientTokens {
                available: self.tokens,
                requested: cost,
            });
        }

        self.tokens -= cost;
        Ok(self.tokens)
    }

    /// Credits bonus tokens back into the bucket.
    pub fn credit_tokens(&mut self, amount: u64) {
        // FLAW (Unit 1 - Broken invariant): allows `tokens > capacity`.
        self.tokens = self.tokens.saturating_add(amount);
    }

    /// Reserves a sequence of individual request costs.
    pub fn reserve_batch(&mut self, costs: &[u64]) -> Result<u64, LimitError> {
        // FLAW (Unit 1 - Partial state mutation on Err):
        // Mutates `self.tokens` eagerly per item without rollback if a later item fails.
        for &cost in costs {
            if cost > self.capacity {
                return Err(LimitError::ExceedsCapacity {
                    capacity: self.capacity,
                    requested: cost,
                });
            }
            if self.tokens < cost {
                return Err(LimitError::InsufficientTokens {
                    available: self.tokens,
                    requested: cost,
                });
            }
            self.tokens -= cost;
        }
        Ok(self.tokens)
    }

    /// Computes milliseconds needed before `cost` tokens are available.
    pub fn effective_wait_ms(&self, cost: u64) -> Result<u64, LimitError> {
        if cost > self.capacity {
            return Err(LimitError::ExceedsCapacity {
                capacity: self.capacity,
                requested: cost,
            });
        }
        if self.tokens >= cost {
            return Ok(0);
        }
        let deficit = cost - self.tokens;
        if self.refill_per_sec == 0 {
            return Ok(u64::MAX);
        }
        // FLAW (Unit 1 - Truncation breaks Hoare postcondition):
        // Integer division rounds down, so waiting this duration can still fail `refill_and_take`.
        Ok((deficit * 1000) / self.refill_per_sec)
    }

    /// Reconciles an external cumulative counter against the local bucket.
    pub fn reconcile_usage(&mut self, reported_total: u64) -> u64 {
        let delta = reported_total.saturating_sub(self.usage_watermark);
        self.tokens = self.tokens.saturating_sub(delta);
        // FLAW (Unit 1 - PIE Defect -> Infection -> Failure):
        // Unconditionally overwriting `usage_watermark` lowers the baseline when an
        // out-of-order report arrives (`reported_total < usage_watermark`), causing the
        // next valid report to double-charge tokens.
        self.usage_watermark = reported_total;
        delta
    }
}

#[cfg(test)]
mod baseline_tests {
    use super::*;

    #[test]
    fn basic_take_and_refill_works() {
        let mut rl = RateLimiter::new(10, 5, 1_000);
        assert_eq!(rl.refill_and_take(1_000, 6), Ok(4));
        // 400ms * 5/sec = 2 tokens refilled -> 6, take 3 -> 3
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
}

#[cfg(test)]
mod lab_goals {
    use super::*;

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
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
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn credit_tokens_preserves_capacity_invariant() {
        let mut rl = RateLimiter::new(10, 0, 0);
        rl.refill_and_take(0, 3).unwrap();
        rl.credit_tokens(20);
        assert!(rl.tokens() <= rl.capacity());
        assert_eq!(rl.tokens(), 10);
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn non_monotonic_timestamp_does_not_corrupt_clock() {
        let mut rl = RateLimiter::new(10, 10, 5_000);
        rl.refill_and_take(5_000, 10).unwrap();
        // Skewed earlier timestamp should neither move last_refill_ms backwards nor grant unearned time.
        assert!(rl.refill_and_take(2_000, 1).is_err());
        assert_eq!(rl.last_refill_ms(), 5_000);
        // At 5_100ms, exactly 100ms elapsed since 5_000ms -> 1 token, not 31 tokens!
        assert_eq!(rl.refill_and_take(5_100, 1), Ok(0));
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn wait_duration_guarantees_subsequent_take_succeeds() {
        let mut rl = RateLimiter::new(10, 3, 1_000);
        rl.refill_and_take(1_000, 10).unwrap();
        let wait = rl.effective_wait_ms(2).unwrap();
        // Postcondition: after waiting `wait` ms, taking 2 tokens MUST succeed.
        assert!(rl.refill_and_take(1_000 + wait, 2).is_ok());
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn stale_reconcile_report_does_not_infect_future_charges() {
        let mut rl = RateLimiter::new(20, 0, 0);
        assert_eq!(rl.reconcile_usage(10), 10);
        // Stale out-of-order reading must be ignored.
        assert_eq!(rl.reconcile_usage(4), 0);
        // Next reading 12 only adds 2 above high-water mark 10, not 8!
        assert_eq!(rl.reconcile_usage(12), 2);
        assert_eq!(rl.tokens(), 8);
    }
}
