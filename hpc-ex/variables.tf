variable "aws_region" {
  description = "AWS Region to deploy resources in"
  type        = string
  default     = "us-east-1"
}

variable "instance_type" {
  description = "EC2 Instance type. Default is t4g.medium (ARM64, 2 vCPUs, 4GB RAM), which complies with cloud sandbox SCP policies while satisfying OpenCHAMI's 4GB RAM requirement."
  type        = string
  default     = "t4g.medium"
}

variable "worker_instance_type" {
  description = "EC2 Instance type. Default is t4g.medium (ARM64, 2 vCPUs, 4GB RAM), which complies with cloud sandbox SCP policies while satisfying OpenCHAMI's 4GB RAM requirement."
  type        = string
  default     = "t4g.medium"
}

variable "root_volume_size" {
  description = "Size of the root EBS volume in GiB (OpenCHAMI tutorial requires at least 60GB)"
  type        = number
  default     = 60
}

variable "root_volume_type" {
  description = "EBS volume type for root disk"
  type        = string
  default     = "gp3"
}

variable "key_pair_name_prefix" {
  description = "Prefix for the generated AWS key pair name"
  type        = string
  default     = "openchami-head-"
}

variable "private_key_filename" {
  description = "Filename for the generated private key stored in the project directory"
  type        = string
  default     = "head_node_key.pem"
}

variable "allowed_ssh_cidrs" {
  description = "List of IPv4 CIDR blocks permitted to connect to the head node via SSH"
  type        = list(string)
  default     = ["0.0.0.0/0"]
}

variable "custom_ami_id" {
  description = "Optional AMI ID override. If omitted, the latest official Rocky Linux 9 AMI matching instance architecture is automatically retrieved."
  type        = string
  default     = null
}

variable "worker_count" {
  description = "Number of worker VMs to deploy into separate subnets"
  type        = number
  default     = 3
}

variable "worker_instance_type" {
  description = "Instance type for worker VMs"
  type        = string
  default     = "t4g.medium"
}

variable "worker_root_volume_size" {
  description = "Root EBS volume size in GiB for worker VMs"
  type        = number
  default     = 30
}

