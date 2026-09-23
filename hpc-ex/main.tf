# --- Dynamic AMI Architecture Resolution ---

locals {
  is_arm           = length(regexall("g\\.", var.instance_type)) > 0
  ami_arch         = local.is_arm ? "arm64" : "x86_64"
  ami_name_pattern = local.is_arm ? "Rocky-9-EC2-Base-*.aarch64*" : "Rocky-9-EC2-Base-*.x86_64*"
}

data "aws_ami" "rocky9" {
  count       = var.custom_ami_id == null ? 1 : 0
  most_recent = true
  owners      = ["792107900819"] # Rocky Enterprise Software Foundation

  filter {
    name   = "name"
    values = [local.ami_name_pattern]
  }

  filter {
    name   = "architecture"
    values = [local.ami_arch]
  }

  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }
}

data "aws_availability_zones" "available" {
  state = "available"
}

locals {
  resolved_ami_id  = var.custom_ami_id != null ? var.custom_ami_id : data.aws_ami.rocky9[0].id
  root_device_name = var.custom_ami_id != null ? "/dev/xvda" : data.aws_ami.rocky9[0].root_device_name
}

# --- SSH Key Pair Generation ---

resource "tls_private_key" "head_node" {
  algorithm = "ED25519"
}

resource "local_sensitive_file" "private_key" {
  content         = tls_private_key.head_node.private_key_openssh
  filename        = "${path.module}/${var.private_key_filename}"
  file_permission = "0600"
}

resource "aws_key_pair" "head_node" {
  key_name_prefix = var.key_pair_name_prefix
  public_key      = tls_private_key.head_node.public_key_openssh
}

# --- Networking (Dedicated Isolated VPC) ---

resource "aws_vpc" "openchami" {
  cidr_block           = "10.0.0.0/16"
  enable_dns_support   = true
  enable_dns_hostnames = true

  tags = {
    Name = "openchami-vpc"
  }
}

resource "aws_internet_gateway" "openchami" {
  vpc_id = aws_vpc.openchami.id

  tags = {
    Name = "openchami-igw"
  }
}

resource "aws_subnet" "public" {
  vpc_id                  = aws_vpc.openchami.id
  cidr_block              = "10.0.1.0/24"
  map_public_ip_on_launch = true

  tags = {
    Name = "openchami-public-subnet"
  }
}

resource "aws_route_table" "public" {
  vpc_id = aws_vpc.openchami.id

  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_internet_gateway.openchami.id
  }

  tags = {
    Name = "openchami-public-rt"
  }
}

resource "aws_route_table_association" "public" {
  subnet_id      = aws_subnet.public.id
  route_table_id = aws_route_table.public.id
}

resource "aws_subnet" "worker" {
  count                   = var.worker_count
  vpc_id                  = aws_vpc.openchami.id
  cidr_block              = cidrsubnet(aws_vpc.openchami.cidr_block, 8, count.index + 2)
  availability_zone       = data.aws_availability_zones.available.names[count.index % length(data.aws_availability_zones.available.names)]
  map_public_ip_on_launch = true

  tags = {
    Name = "openchami-worker-subnet-${count.index + 1}"
    Role = "worker"
  }
}

resource "aws_route_table_association" "worker" {
  count          = var.worker_count
  subnet_id      = aws_subnet.worker[count.index].id
  route_table_id = aws_route_table.public.id
}

# --- Security Group & Decoupled Rules (AWS Provider v6) ---

resource "aws_security_group" "head_node" {
  name_prefix = "openchami-head-sg-"
  description = "Security group for OpenCHAMI head node"
  vpc_id      = aws_vpc.openchami.id

  tags = {
    Name = "openchami-head-sg"
  }
}

resource "aws_vpc_security_group_ingress_rule" "ssh" {
  for_each          = toset(var.allowed_ssh_cidrs)
  security_group_id = aws_security_group.head_node.id
  description       = "Inbound SSH access"
  cidr_ipv4         = each.value
  from_port         = 22
  to_port           = 22
  ip_protocol       = "tcp"
}

resource "aws_vpc_security_group_ingress_rule" "vpc_internal" {
  security_group_id = aws_security_group.head_node.id
  description       = "All internal traffic within VPC"
  cidr_ipv4         = aws_vpc.openchami.cidr_block
  ip_protocol       = "-1"
}

resource "aws_vpc_security_group_egress_rule" "all_outbound" {
  security_group_id = aws_security_group.head_node.id
  description       = "All outbound traffic"
  cidr_ipv4         = "0.0.0.0/0"
  ip_protocol       = "-1"
}

# --- Launch Template & EC2 Instance ---

resource "aws_launch_template" "head_node" {
  name_prefix   = "openchami-head-template-"
  image_id      = local.resolved_ami_id
  instance_type = var.instance_type
  key_name      = aws_key_pair.head_node.key_name

  vpc_security_group_ids = [aws_security_group.head_node.id]

  block_device_mappings {
    device_name = local.root_device_name

    ebs {
      volume_size           = var.root_volume_size
      volume_type           = var.root_volume_type
      delete_on_termination = true
    }
  }

  user_data = filebase64("${path.module}/cloud-init.yaml")

  tag_specifications {
    resource_type = "instance"
    tags = {
      Name = "openchami-head-node"
    }
  }

  tag_specifications {
    resource_type = "volume"
    tags = {
      Name = "openchami-head-node-root"
    }
  }
}

resource "aws_instance" "head_node" {
  launch_template {
    id      = aws_launch_template.head_node.id
    version = "$Latest"
  }

  subnet_id = aws_subnet.public.id

  tags = {
    Name = "openchami-head-node"
  }
}

# --- Worker Launch Template & EC2 Instances ---

resource "aws_launch_template" "worker" {
  name_prefix   = "openchami-worker-template-"
  image_id      = local.resolved_ami_id
  instance_type = var.worker_instance_type
  key_name      = aws_key_pair.head_node.key_name

  vpc_security_group_ids = [aws_security_group.head_node.id]

  block_device_mappings {
    device_name = local.root_device_name

    ebs {
      volume_size           = var.worker_root_volume_size
      volume_type           = var.root_volume_type
      delete_on_termination = true
    }
  }

  user_data = filebase64("${path.module}/cloud-init.yaml")

  tag_specifications {
    resource_type = "instance"
    tags = {
      Name = "openchami-worker"
      Role = "worker"
    }
  }

  tag_specifications {
    resource_type = "volume"
    tags = {
      Name = "openchami-worker-root"
      Role = "worker"
    }
  }
}

resource "aws_instance" "worker" {
  count = var.worker_count

  launch_template {
    id      = aws_launch_template.worker.id
    version = "$Latest"
  }

  subnet_id = aws_subnet.worker[count.index].id

  tags = {
    Name = "openchami-worker-${count.index + 1}"
    Role = "worker"
  }
}

