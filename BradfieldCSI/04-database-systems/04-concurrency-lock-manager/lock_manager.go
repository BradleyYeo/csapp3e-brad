package lock_manager

type LockType string

const (
	Shared    LockType = "S"
	Exclusive LockType = "X"
)

type Lock struct {
	Type     LockType
	Resource string
}

type Transaction struct {
	Id        int
	HeldLocks []*Lock
}
